#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <menu.h>

#include "settings.h"
#include "scan.h"
#include "term.h"
#include "utils.h"

int main(int argc, char** argv)
{
    char* path = get_application_path();

    char buf[256];

    snprintf(
        buf,
        sizeof(buf),
        "nohup \"rxvt-unicode -e %s\" &",
        path
    );

    if ( ! running_from_term()) {
        // system("rxvt-unicode -e /home/lchsk/projects/xstarter/bin/xstarter");
        // system("rxvt-unicode -e ./xstarter");
        system(buf);
        return 0;
    } else {
        load_config();
        load_cache();

        init_search();
        init_term_gui();

        run_term();

        free_term_gui();
        free_cache();
        free_search();
        free_config();

        open_app();
    }
}
