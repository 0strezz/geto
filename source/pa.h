#ifndef PARARG_PARARG_H
#define PARARG_PARARG_H

/* Specifies if a determinated flag takes arguments:
 * pa_takes_arg: --flag [arg-must-be-here]
 *           or: --flag=arg-must-here
 *
 * pa_might_arg: --flag [arg-might-be]
 *           or: --flag=arg-might-be
 *           or: --flag
 *
 * pa_noway_arg: --flag
 */
enum pa_takes
{
    pa_takes_arg,
    pa_might_arg,
    pa_noway_arg
};

/* `pa_get` function will always return any of these
 * or the id of the last flag seen
 */
enum pa_return
{
    /* External usage, the caller function
     * uses these enums to handle errors
     */
    pa_ret_cest_fini    =  0,
    pa_ret_pos_arg      = -1,
    pa_ret_undef_flag   = -2,
    pa_ret_missed_arg   = -3,
    pa_ret_nonsense     = -4,
    /* Internal usage, pa.c uses these enums
     * to know what is the nature of each
     * element in argv
     */
    pa_ret_argument     = -5,
    pa_ret_1s_flag      = -6,
    pa_ret_2s_flag      = -7,
};

struct pa_option
{
    char  *flag;
    char  id;
    enum  pa_takes takes;
};

struct pa_similar
{
    char *flag;
    unsigned char fav;
};

/* This variable stores the value of the last argument given
 */
extern char *pa_argument;

/* This variable stores the name of the last flag used
 */
extern char *pa_flagname;

/* Migth the programmer does not want to allow
 * unix style for providing arguments.
 * Allowed by default.
 */
extern char pa_unixstyle_allowed;

/* If this flag is on and a flagname is not given
 * correctly, pa will create a list of similar
 * flagnames.
 * Allowed by default.
 */
extern char pa_do_fuzzy_matching;

/* List of most similiar flags, use it only if
 * pa_ret_undef_flag is returned and pa_do_fuzzy_matching
 * is on
 */
extern struct pa_similar *pa_similar;

/* This is not the best name, but it's good enough.
 * This can either be an error, or the id of the
 * last argument given.
 */
typedef signed char pa_t;

pa_t pa_get (const unsigned int, char**, unsigned short*, const unsigned short, const struct pa_option*);

#endif
