#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>
#include <menu.h>

#include "settings.h"
#include "scan.h"
#include "term.h"
#include "utils.h"

int
main(int argc, char **argv)
{
    set_err(NO_ERR);

    xstarter_directory();

    cmdline_t *cmdline = smalloc(sizeof(cmdline_t));

    if (read_cmdline(cmdline, argc, argv))
        exit(EXIT_SUCCESS);

    if (cmdline->mode == MODE_RETURN_TERMINAL) {
        load_config(cmdline);
        char *terminal = config()->section_main->terminal;
        printf("%s", terminal);
        free_config();
    } else if (cmdline->mode == MODE_OPEN_APP) {
        load_config(cmdline);

        init_search();
        init_term_gui();

        load_cache(cmdline);

        run_term();

        kill_scan();
        free_term_gui();
        free_cache();
        free_search();
        free_config();

        open_app();
    } else {
        set_err(ERR_UNKNOWN_APP_MODE);
    }

    if (cmdline->verbose) {
        print_err();
    }

    free_cmdline(cmdline);

    return get_err();
}
