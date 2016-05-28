#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "utils.h"

static char* _app_to_open_path;

static void
record_open_file(const char* path)
{
    if (xstarter_dir_avail) {
        char recently_f[1024];

        snprintf(recently_f, sizeof(recently_f), "%s/recent", xstarter_dir);

        FILE* file = fopen(recently_f, "wb");

        if (file != NULL) {
            // Check if it's already in the list

            int path_index = -1;

            int items_to_save = recent_apps_cnt + 1;

            for (int i = 0; i < recent_apps_cnt; i++) {
                if (strcmp(recent_apps[i].path, path) == 0) {
                    path_index = i;
                    items_to_save -= 1;
                }
            }

            int last_index = recent_apps_cnt;

            if (last_index >= RECENT_APPS_REMEMBERED)
                last_index = recent_apps_cnt - 1;

            // Remove item already existing in the list

            if (path_index != -1) {
                for (int i = path_index; i < last_index; i++) {
                    strcpy(recent_apps[i].path, recent_apps[i + 1].path);
                }
            }

            // Move rest of the items

            for (int i = last_index; i > 0; i--) {
                strcpy(recent_apps[i].path, recent_apps[i - 1].path);
            }

            strcpy(recent_apps[0].path, path);

            fwrite(
                &recent_apps,
                sizeof(recently_open_t),
                items_to_save,
                file
                );

            fclose(file);
        }
    }
}

void
read_recently_open_list()
{
    recent_apps_cnt = 0;

    if (xstarter_dir_avail) {
        FILE* fptr;
        char recently_f[1024];

        snprintf(recently_f, sizeof(recently_f), "%s/recent", xstarter_dir);

        fptr = fopen(recently_f, "rb");

        if (fptr) {
            recent_apps_cnt = fread(
                recent_apps,
                sizeof(recently_open_t),
                RECENT_APPS_REMEMBERED,
                fptr
                );

            fclose(fptr);
        }
    }
}

void open_app(const int mode)
{
	if (_app_to_open_path) {
        record_open_file(_app_to_open_path);

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

/* int */
/* get_config_path(char* home_dir) */
/* { */
/* 	char* dir = NULL; */
/* 	if ((dir = (getenv("HOME"))) == NULL) { */
/* 		struct passwd* pw = getpwuid(getuid()); */
/* 		dir = pw->pw_dir; */
/* 	} */

/* 	if (dir != NULL) { */
/* 		strcpy(home_dir, dir); */
/* 		return 0; */
/* 	} */

/* 	return 1; */
/* } */

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

void
dump_debug_int(int d)
{
	char debug[1024];

    snprintf(
        debug,
        1024,
        "echo %d>> ~/debug_xstarter",
        d
    );

	system(debug);
}

void
get_working_dir()
{
    // TODO:
    // readlink("/proc/curproc/file", buf, bufsize) (FreeBSD)
    // readlink("/proc/self/path/a.out", buf, bufsize) (Solaris)
    // argv[0]

    /* char app[1000]; */

    /* if (readlink("/proc/self/exe", app, sizeof(app)) == -1) { */
    /*     // TODO */
    /* } */

    /* strcpy(working_dir, dirname(app)); */

    /* char recent_f[1200]; */

    /* snprintf(recent_f, sizeof(recent_f), "%s/recent.bin", working_dir); */
}

void
xstarter_directory()
{
    xstarter_dir_avail = True;

	char* dir = NULL;

	if ((dir = (getenv("HOME"))) == NULL) {
		struct passwd* pw = getpwuid(getuid());
		dir = pw->pw_dir;
	}

    if (! dir) {
        // TODO: Set dir = /tmp
    }

	if (dir) {
        snprintf(
            xstarter_dir,
            sizeof(xstarter_dir),
            "%s/.xstarter.d",
            (dir)
            );

        struct stat st = {0};

        if (stat(xstarter_dir, &st) == -1) {
            if (mkdir(xstarter_dir, 0700)) {
                // TODO: set error code
            } else {
                xstarter_dir_avail = True;
            }
        } else {
            xstarter_dir_avail = True;
        }
	}
}
