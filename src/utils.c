#include <unistd.h>
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* _app_to_open_path;

void open_app(const int mode)
{
    if (_app_to_open_path) {
		char buf[256];

		if (mode == MODE_SAVE_TO_FILE) {
		  	snprintf(
				 buf,
				 sizeof(buf),
				 "echo %s> /tmp/.xstarter",
				 _app_to_open_path
			);
		}
		else if (mode == MODE_OPEN_IMMEDIATELY) {
			snprintf(
				 buf,
				 sizeof(buf),
				 "nohup %s 2> /dev/null &",
				 _app_to_open_path
			);

		}

		system(buf);
    }
}

void
app_to_open(char* path)
{
    _app_to_open_path = path;
}

int
running_from_term()
{
    return isatty(0);
}
