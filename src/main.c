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
	/* char* path = get_application_path(); */
	int error = 0;

	cmdline_t* cmdline = malloc(sizeof(cmdline_t));
	read_cmdline(cmdline, argc, argv);

	if (cmdline->help == 1) {
		usage();
	} else if (cmdline->mode == MODE_RETURN_TERMINAL) {
		load_config();
		char* terminal = config()->section_main->terminal;
		printf("%s", terminal);
		free_config();
	} else if (
		cmdline->mode == MODE_OPEN_IMMEDIATELY
		|| cmdline->mode == MODE_SAVE_TO_FILE
	) {
		load_config();
		load_cache();

		init_search();
		init_term_gui();

		run_term();

		free_term_gui();
		free_cache();
		free_search();
		free_config();

		open_app(cmdline->mode);
	} else {
		printf("Unknown application mode");
		error = 1;
	}

	if (cmdline) {
		free(cmdline);
	}

	return error;
}
