#include "pa.h"
#include <stdio.h>
#include <stdlib.h>

static void list_similar (const char*);

int main (int argc, char **argv)
{
    static const struct pa_option opts[] =
    {
        {"takes", 't', pa_takes_arg},
        {"might", 'm', pa_might_arg},
        {"noway", 'n', pa_noway_arg},
        {"add", 'x', pa_takes_arg},
        {"am", 'x', pa_takes_arg},
        {"apply", 'x', pa_takes_arg},
        {"archive", 'x', pa_takes_arg},
        {"bisect", 'x', pa_takes_arg},
        {"blame", 'x', pa_takes_arg},
        {"branch", 'x', pa_takes_arg},
        {"bundle", 'x', pa_takes_arg},
        {"checkout", 'x', pa_takes_arg},
        {"cherry", 'x', pa_takes_arg},
        {"cherry-pick", 'x', pa_takes_arg},
        {"citool", 'x', pa_takes_arg},
        {"clean", 'x', pa_takes_arg},
        {"clone", 'x', pa_takes_arg},
        {"commit", 'x', pa_takes_arg},
        {"config", 'x', pa_takes_arg},
        {"describe", 'x', pa_takes_arg},
        {"diff", 'x', pa_takes_arg},
        {"difftool", 'x', pa_takes_arg},
        {"fetch", 'x', pa_takes_arg},
        {"format-patch", 'x', pa_takes_arg},
        {"fsck", 'x', pa_takes_arg},
        {"gc", 'x', pa_takes_arg},
        {"gitk", 'x', pa_takes_arg},
        {"grep", 'x', pa_takes_arg},
        {"gui", 'x', pa_takes_arg},
        {"help", 'x', pa_takes_arg},
        {"init", 'x', pa_takes_arg},
        {"instaweb", 'x', pa_takes_arg},
        {"log", 'x', pa_takes_arg},
        {"maintenance", 'x', pa_takes_arg},
        {"merge", 'x', pa_takes_arg},
        {"mergetool", 'x', pa_takes_arg},
        {"mv", 'x', pa_takes_arg},
        {"notes", 'x', pa_takes_arg},
        {"prune", 'x', pa_takes_arg},
        {"pull", 'x', pa_takes_arg},
        {"push", 'x', pa_takes_arg},
        {"range-diff", 'x', pa_takes_arg},
        {"rebase", 'x', pa_takes_arg},
        {"reflog", 'x', pa_takes_arg},
        {"remote", 'x', pa_takes_arg},
        {"repack", 'x', pa_takes_arg},
        {"replace", 'x', pa_takes_arg},
        {"request-pull", 'x', pa_takes_arg},
        {"reset", 'x', pa_takes_arg},
        {"restore", 'x', pa_takes_arg},
        {"revert", 'x', pa_takes_arg},
        {"rm", 'x', pa_takes_arg},
        {"send-email", 'x', pa_takes_arg},
        {"shortlog", 'x', pa_takes_arg},
        {"show", 'x', pa_takes_arg},
        {"show-branch", 'x', pa_takes_arg},
        {"sparse-checkout", 'x', pa_takes_arg},
        {"stage", 'x', pa_takes_arg},
        {"stash", 'x', pa_takes_arg},
        {"status", 'x', pa_takes_arg},
        {"submodule", 'x', pa_takes_arg},
        {"switch", 'x', pa_takes_arg},
        {"tag", 'x', pa_takes_arg},
        {"whatchanged", 'x', pa_takes_arg},
        {"worktree", 'x', pa_takes_arg},
    };

    unsigned short a;

    while (1)
    {
        pa_t ret = pa_get(argc, argv, &a, 65, opts);
        if (ret == pa_ret_cest_fini) { break; }

        switch (ret)
        {
            case 't':
            {
                printf("Takes took (%s) : %s\n", pa_flagname, pa_argument);
                break;
            }
            case 'm':
            {
                printf("Might took? (%s): %s <%s>\n", pa_flagname, pa_argument == NULL ? "no" : "yes", pa_argument);
                break;
            }
            case 'n':
            {
                printf("No way was seen! (%s)\n", pa_flagname);
                break;
            }
            case pa_ret_pos_arg:
            {
                printf("pos argument given: %s\n", pa_argument);
                break;
            }
            case pa_ret_undef_flag:
            {
                printf("undeflag! %s\n", argv[a]);
                list_similar(argv[a]);
                break;
            }
            case pa_ret_missed_arg:
            {
                printf("missed argument for `%s` flag\n", pa_flagname);
                break;
            }
            case pa_ret_nonsense:
            {
                printf("wtf '%s' does even mean?\n", argv[a]);
                break;
            }
        }
    }

    return 0;
}

static void list_similar (const char *flagname)
{
    fprintf(stderr, "PA: `%s` flag is not defined\n", flagname);
    fprintf(stderr, "    did you mean:\n");

    for (unsigned int i = 0; pa_similar[i].flag; i++)
    {
        fprintf(stderr, "    * \x1b[%dm%s\x1b[0m\n", pa_similar[i].fav ? 1 : 0, pa_similar[i].flag);
    }
}
