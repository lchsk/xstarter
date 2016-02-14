#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <menu.h>

#include "settings.h"
#include "scan.h"
#include "term.h"
#include "utils.h"

int main()
{
    if ( ! running_from_term()) {
        system("rxvt-unicode -e /home/lchsk/projects/xstarter/bin/xstarter");
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

