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
main(int argc, char** argv)
{
    int error = 0;

    xstarter_directory();

    cmdline_t* cmdline = malloc(sizeof(cmdline_t));

    if (read_cmdline(cmdline, argc, argv))
        exit(EXIT_SUCCESS);

    if (cmdline->mode == MODE_RETURN_TERMINAL) {
        load_config(cmdline);
        char* terminal = config()->section_main->terminal;
        printf("%s", terminal);
        free_config();
    } else if (cmdline->mode == MODE_OPEN_APP) {
        load_config(cmdline);

        init_search();
        init_term_gui();

        load_cache();

        run_term();

        free_term_gui();
        free_cache();
        free_search();
        free_config();

        open_app();
    } else {
        printf("Unknown application mode");
        error = 1;
    }

    free_cmdline(cmdline);

    return error;
}
