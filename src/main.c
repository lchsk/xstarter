#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include "settings.h"
#include "scan.h"
#include "term.h"
#include "utils.h"

int main()
{
    load_config();

    load_cache();

    init_search();

    init_term_gui();
    run_term();
    free_term_gui();

    free_search();
    free_cache();
    free_config();

    // if ( ! running_from_term()) {
    //     popen("rxvt-unicode -e /home/lchsk/projects/xstarter/bin/xstarter", "w");
    //     return 0;
    // }

  // popen("firefox", "w");
}

