#define _GNU_SOURCE // execvpe
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // umask
#include <unistd.h>   // execvpe, readlink

#include <glib.h>

#include "scan.h"
#include "settings.h"
#include "term.h"
#include "utils.h"

int main(int argc, char **argv)
{
    set_err(NO_ERR);
    xstarter_directory();
    init_search();

    CmdLine *cmdline = smalloc(sizeof(CmdLine));

    if (cmdline_read(cmdline, argc, argv))
        exit(EXIT_FAILURE);

    config_load(cmdline);

    if (cmdline->print_list_of_cache_apps) {
        // Print applications from cache; to be used by external programs
        load_cache(cmdline, /* extra_thread */ false);
        print_cache_apps();

        exit(EXIT_SUCCESS);
    }

    const Config *conf = config_get();

    str_copy(exec_term, conf->section_main->terminal, sizeof(exec_term));

    if (! in_terminal()) {
        cmdline_free(cmdline);
        config_free(config_get());

        open_itself(argc, argv);
    }

    init_term_gui();

    load_cache(cmdline, /* extra_thread */ true);

    run_term();

    kill_scan();
    free_cache();
    free_search();
    config_free(config_get());

    if (cmdline->verbose) {
        print_err();
    }

    cmdline_free(cmdline);

    return get_err();
}
