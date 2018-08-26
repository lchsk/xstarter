#define _GNU_SOURCE // execvpe
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // execvpe, readlink
#include <sys/stat.h> // umask
#include <errno.h>

#include <glib.h>

#include "settings.h"
#include "scan.h"
#include "term.h"
#include "utils.h"

int main(int argc, char **argv)
{
    set_err(NO_ERR);

    xstarter_directory();

    cmdline_t *cmdline = smalloc(sizeof(cmdline_t));

    if (read_cmdline(cmdline, argc, argv))
        exit(EXIT_FAILURE);

    load_config(cmdline);

    if (cmdline->print_list_of_cache_apps) {
        // Print applications from cache; to be used by external programs
        load_cache(cmdline, /* extra_thread */ false);
        print_cache_apps();

        exit(EXIT_SUCCESS);
    }

    const config_t *conf = config();

    strncpy(exec_term, conf->section_main->terminal, sizeof(exec_term));

    if (! in_terminal()) {
        free_cmdline(cmdline);

        free_config();

        open_itself(argc, argv);
    }

    init_search();
    init_term_gui();

    load_cache(cmdline, /* extra_thread */ true);

    run_term();

    kill_scan();
    free_cache();
    free_search();
    free_config();

    if (cmdline->verbose) {
        print_err();
    }

    free_cmdline(cmdline);

    return get_err();
}
