#include <unistd.h>
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* _app_to_open_path;

void open_app()
{
    if (_app_to_open_path) {

        // Buffer for the command
        char buf[256];

		snprintf(
				 buf,
				 sizeof(buf),
				 "echo %s> /tmp/.xstarter",
				 _app_to_open_path
				 );
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
