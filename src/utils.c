#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static char* _app_to_open_path;

void open_app(const int mode)
{
	if (_app_to_open_path) {
		char command[256];

		if (mode == MODE_SAVE_TO_FILE) {
			snprintf(
				command,
				sizeof(command),
				"echo %s> /tmp/.xstarter",
				_app_to_open_path
			);
		} else if (mode == MODE_OPEN_IMMEDIATELY) {
			snprintf(
				command,
				sizeof(command),
				"nohup %s 2> /dev/null &",
				_app_to_open_path
			);
		}

		system(command);
	}
}

void
app_to_open(char* path)
{
	_app_to_open_path = path;
}

/* TODO: remove */
int
running_from_term()
{
	return isatty(0);
}

int
get_config_path(char* home_dir)
{
	char* dir = NULL;
	if ((dir = (getenv("HOME"))) == NULL) {
		struct passwd* pw = getpwuid(getuid());
		dir = pw->pw_dir;
	}

	if (dir != NULL) {
		strcpy(home_dir, dir);
		return 0;
	}

	return 1;
}

void
dump_debug(const char* str)
{
	char debug[1024];

    snprintf(
        debug,
        1024,
        "echo %s>> ~/debug_xstarter",
        str
    );

	system(debug);
}
