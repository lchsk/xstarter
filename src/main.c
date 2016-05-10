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

int main(int argc, char** argv)
{
	int c;
	int mode = MODE_OPEN_IMMEDIATELY;

	while ((c = getopt(argc, argv, "tp")) != -1) {
		switch(c) {
		case 't':
			printf("rxvt-unicode");
			return 0;
		case 'p':
			mode = MODE_SAVE_TO_FILE;
			break;
		}
	}

    char* path = get_application_path();

    load_config();
    load_cache();

    init_search();
    init_term_gui();

    run_term();

    free_term_gui();
    free_cache();
    free_search();
    free_config();

    open_app(mode);
}
