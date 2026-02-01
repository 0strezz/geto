#include "pa.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define CHECK_ALLOC(ptr)                                   \
    do {                                                   \
        if (ptr) break;                                    \
        fprintf(stderr, "PA: Cannot allocate memory\n");   \
        exit(EXIT_FAILURE);                                \
    } while (0)

#define MAX(a, b)          ((a) > (b) ? (a) : (b))
#define MIN(a, b)          ((a) < (b) ? (a) : (b))

#define PATH_SPECIFIER(a)  ((a == '~') || (a == '.') || (a == '/'))

/* Extern defs.
 */
char *pa_argument = NULL;
char *pa_flagname = NULL;

char pa_unixstyle_allowed = 1;
char pa_do_fuzzy_matching = 1;

struct pa_similar *pa_similar = NULL;

/* This array stores the length of every flagname provided
 * in the options, this is made in order to not recalculate
 * them every time PA is looking for an flag.
 */
static unsigned int *FlagLengths = NULL;

/* Executable name aka argv[0]
 */
static char *CallerName;

static void check_options_are_ok (const struct pa_option*, const unsigned short);
static enum pa_return kind_of_thing (const char*);

static enum pa_return handle_1s (const char, const unsigned short, char*, const struct pa_option*);
static enum pa_return handle_2s (const char*, char*, const unsigned short, const struct pa_option*);

static enum pa_return check_flag (const enum pa_takes, char*, const char);
static unsigned int unix_like (const char*, unsigned int*);

static void do_fuzzy_matching (const char*, const unsigned short, const struct pa_option*);
static double LevenshteinThreshold (const char*, const char*, const unsigned int, const unsigned int);

pa_t pa_get (const unsigned int argc, char **argv, unsigned short *index, const unsigned short nopts, const struct pa_option *opts)
{
    if (FlagLengths == NULL)
    {
        check_options_are_ok(opts, nopts);
        CallerName = *argv;
    }

    /* `at` is an index to know what element is being read
     * from argv, this is also a way to let the programmer
     * to know what was the last string read in argv
     */
    static unsigned short at = 1;
    *index = at;

    while (at < argc)
    {
        char *thing = argv[at++], *next = (at < argc) ? argv[at] : NULL;
        pa_argument = NULL;

        switch (kind_of_thing(thing))
        {
            case pa_ret_nonsense: { return pa_ret_nonsense; }

            case pa_ret_1s_flag:
            {
                const enum pa_return ret = handle_1s(*(thing + 1), nopts, next, opts);
                if (pa_argument != NULL) { at++; }
                return ret;
            }

            case pa_ret_2s_flag:
            {

                const enum pa_return ret = handle_2s(thing + 2, next, nopts, opts);
                if (pa_argument != NULL) { at++; }
                return ret;
            }

            case pa_ret_argument:
            {
                pa_argument = thing;
                return pa_ret_pos_arg;
            }
        }
    }

    if (FlagLengths) { free(FlagLengths); }
    return pa_ret_cest_fini;
}

static void check_options_are_ok (const struct pa_option *opts, const unsigned short nopts)
{
    FlagLengths = (unsigned int*) calloc(nopts, sizeof(*FlagLengths));
    CHECK_ALLOC(FlagLengths);

    char *errReason = NULL;
    unsigned short opterr;

    for (unsigned int i = 0; i < nopts; i++)
    {
        if (opts[i].flag == NULL)
        {
            errReason = "Flagnames cannot be null nor start with spaces";
            opterr = i;
            break;
        }
        if (!isalnum(opts[i].id) || !isalnum(opts[i].flag[0]))
        {
            errReason = "Both id and flagnames must only contain [0-9] [a-z] or [A-Z]";
            opterr = i;
            break;
        }

        FlagLengths[i] = (unsigned int) strlen(opts[i].flag);
        if (FlagLengths[i] == 0)
        {
            errReason = "Flagnames cannot be empty";
            opterr = i;
            break;
        }
    }

    if (errReason != NULL)
    {
        static const char *const err =
         "\n"
         "  [PA:%s]: cannot continue...\n"
         "   due to: %s\n"
         "       at: %dth option\n"
         "\n";
        fprintf(stderr, err, CallerName, errReason, ++opterr);
        exit(EXIT_FAILURE);
    }
}

static enum pa_return kind_of_thing (const char *thing)
{
    if (thing == NULL)
    {
        return pa_ret_nonsense;
    }
    if (*thing == '-' && isalnum(*(thing + 1)) && *(thing + 2) == 0)
    {
        return pa_ret_1s_flag;
    }
    if (*thing == '-' && *(thing + 1) == '-' && isalnum(*(thing + 2)))
    {
        return pa_ret_2s_flag;
    }
    if (isalnum(*thing) || PATH_SPECIFIER(*thing))
    {
        return pa_ret_argument;
    }
    return pa_ret_nonsense;
}


static enum pa_return handle_1s (const char id, const unsigned short nopts, char *next, const struct pa_option *opts)
{
    for (unsigned short i = 0; i < nopts; i++)
    {
        if (opts[i].id != id) { continue; }
        pa_flagname = opts[i].flag;
        return check_flag(opts[i].takes, next, id);
    }
    return pa_ret_undef_flag;
}

static enum pa_return handle_2s (const char *flag, char *next, const unsigned short nopts, const struct pa_option *opts)
{
    for (unsigned short i = 0; i < nopts; i++)
    {
        if (strncmp(flag, opts[i].flag, FlagLengths[i])) { continue; }
        pa_flagname = opts[i].flag;

        unsigned int argStartsAt, diff = 0;
        if (pa_unixstyle_allowed == 1)
        {
            diff = unix_like(flag, &argStartsAt);
        }
        if (diff != 0)
        {
            if (opts[i].takes == pa_noway_arg) { return diff ? pa_ret_nonsense : opts[i].id; }
            if (opts[i].takes == pa_might_arg && diff == 0) { return opts[i].id; }
            if (opts[i].takes == pa_takes_arg && diff == 0) { return pa_ret_missed_arg; }

            pa_argument = (char*) calloc(diff, sizeof(char));
            CHECK_ALLOC(pa_argument);

            memcpy(pa_argument, flag + argStartsAt, diff);
            return opts[i].id;
        }

        return check_flag(opts[i].takes, next, opts[i].id);
    }

    if (pa_do_fuzzy_matching) { do_fuzzy_matching(flag, nopts, opts); }
    return pa_ret_undef_flag;
}

static enum pa_return check_flag (const enum pa_takes takes, char *next, const char id)
{
    const enum pa_return nexts = kind_of_thing(next);

    if (takes == pa_takes_arg)
    {
        if (nexts != pa_ret_argument) { return pa_ret_missed_arg; }
        pa_argument = next;
        return id;
    }
    if (takes == pa_might_arg)
    {
        if (nexts != pa_ret_argument) { return id; }
        pa_argument = next;
        return id;
    }

    return id;
}

static unsigned int unix_like (const char *thing, unsigned int *argStartsAt)
{
    /* Suppose this scenario:
     * --flag=a
     * 01234567
     *
     * This whole element in argv has a length
     * of 16 bytes, the equal sign is found at
     * the seventh byte, what this function
     * returns is the difference between the whole
     * legth and the position where `=` was found
     * plus one, if no `=` is found the diff is gonna
     * be zero since `i` reaches the end
     */
    unsigned int i = 0, len = strlen(thing);
    for (; i < len; i++)
    {
        if (thing[i] == '=')
        {
            if (argStartsAt) { *argStartsAt = ++i; }
            return len - i;
        }
    }

    if (argStartsAt) { *argStartsAt = 0; }
    return 0;
}

static void do_fuzzy_matching (const char *flag, const unsigned short nopts, const struct pa_option *opts)
{
    pa_similar = (struct pa_similar*) calloc(nopts + 1, sizeof(*pa_similar));
    CHECK_ALLOC(pa_similar);

    const unsigned int unixlike = unix_like(flag, NULL);
    const unsigned int flaglen = (unixlike == 0) ? (unsigned int) strlen(flag) : unixlike - 1;

    for (unsigned int i = 0, j = 0; i < nopts; i++)
    {
        const double threshold = LevenshteinThreshold(flag, opts[i].flag, flaglen, FlagLengths[i]);

        if (threshold >= 0.2f)
        {
            pa_similar[j].flag  = (char*) opts[i].flag;
            pa_similar[j++].fav = (threshold >= 0.5f) ? 1 : 0;
        }
    }
}

static double LevenshteinThreshold (const char *s1, const char *s2, const unsigned int l1, const unsigned int l2)
{
    unsigned short *row_0 = (unsigned short*) calloc(l1 + 1, sizeof(unsigned short)),
                   *row_1 = (unsigned short*) calloc(l1 + 1, sizeof(unsigned short));

    CHECK_ALLOC(row_0); CHECK_ALLOC(row_1);
    for (unsigned short i = 0; i <= l1; i++)
    {
        row_0[i] = i;
    }
    for (unsigned short i = 1; i <= l2; i++)
    {
        row_1[0] = i;
        for (unsigned short j = 1; j <= l1; j++)
        {
            const unsigned short r = row_0[j - 1], d = row_0[j], I = row_1[j - 1];
            if (s1[j - 1] != s2[i - 1])
            {
                row_1[j] = 1 + MIN(r, MIN(d, I));
            }
            else
            {
                row_1[j] = r;
            }
        }
        memcpy(row_0, row_1, sizeof(*row_1) * (l1 + 1));
        memset(row_1, 0, sizeof(*row_1) * (l1 + 1));
    }

    const unsigned short steps = row_0[l1];
    free(row_0);
    free(row_1);

    return 1 - ((double) steps / MAX(l1, l2));
}
