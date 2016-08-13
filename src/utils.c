#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "utils.h"

#define PIPE "/tmp/xstarter"

static char* _app_to_open_path;

static void
record_open_file(const char* path)
{
    if (xstarter_dir_avail) {
        char recently_f[1024];

        snprintf(recently_f, sizeof(recently_f), "%s/recent", xstarter_dir);

        FILE *file = fopen(recently_f, "w");

        if (file != NULL) {
            // Check if it's already in the list

            int path_index = -1;

            int items_to_save = recent_apps_cnt + 1;

            for (int i = 0; i < recent_apps_cnt; i++) {
                if (strcmp(recent_apps[i], path) == 0) {
                    path_index = i;
                    items_to_save -= 1;
                }
            }

            int last_index = recent_apps_cnt;

            if (last_index >= RECENT_APPS_REMEMBERED) {
                last_index = recent_apps_cnt - 1;
                items_to_save = RECENT_APPS_REMEMBERED;
            }

            // Remove item already existing in the list

            if (path_index != -1) {
                for (int i = path_index; i < last_index; i++) {
                    strcpy(recent_apps[i], recent_apps[i + 1]);
                }
            }

            // Move rest of the items

            for (int i = last_index; i > 0; i--) {
                strcpy(recent_apps[i], recent_apps[i - 1]);
            }

            strcpy(recent_apps[0], path);

            for (int i = 0; i < items_to_save; i++) {
                fprintf(file, "%s\n", recent_apps[i]);
            }

            fclose(file);
        }
    }
}

void
read_recently_open_list()
{
    recent_apps_cnt = 0;

    if (xstarter_dir_avail) {
        FILE *fptr;
        char recently_f[1024];

        snprintf(recently_f, sizeof(recently_f), "%s/recent", xstarter_dir);

        fptr = fopen(recently_f, "r");

        if (fptr) {
            char line[1024];

            int i = 0;

            while(fgets(line, 1024, fptr)) {
                memmove(recent_apps[i], line, strlen(line) - 1);
                i++;
            }

            recent_apps_cnt = i;

            fclose(fptr);
        }
    }
}

void open_app()
{
    if (_app_to_open_path) {
        record_open_file(_app_to_open_path);

        char command[256];

        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int pipe = open(PIPE, O_RDONLY | O_CREAT);
        char pipe_data[2];

        int mod = read(pipe, pipe_data, 1);

        if (mod == 0) {
            // Open
            snprintf(
                command,
                sizeof(command),
                "nohup %s 2> /dev/null &",
                _app_to_open_path
            );
            system(command);

        } else {
            // save to pipe
            char line[1024];

            int pipe = open(
                PIPE,
                O_WRONLY | O_CREAT | O_TRUNC, mode
            );

            strcpy(line, _app_to_open_path);

            write(pipe, line, strlen(line) + 1);

            close(pipe);
        }
    }
}

void
app_to_open(char *path)
{
    _app_to_open_path = path;
}

void
dump_debug(const char *str)
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
dump_debug_char(const char c)
{
    char debug[1024];

    snprintf(
        debug,
        1024,
        "echo %c>> ~/debug_xstarter",
        c
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
        "echo %d >> ~/debug_xstarter",
        d
    );

    system(debug);
}

void
xstarter_directory()
{
    xstarter_dir_avail = True;
    Boolean using_tmp_dir = False;

    char *dir = NULL;

    if ((dir = (getenv("HOME"))) == NULL) {
        struct passwd *pw = getpwuid(getuid());
        dir = pw->pw_dir;
    }

    if (! dir) {
        using_tmp_dir = True;
        dir = smalloc(1024);
        strcpy(dir, "/tmp");
    }

    if (dir) {
        snprintf(
            xstarter_dir,
            sizeof(xstarter_dir),
            "%s/.xstarter.d",
            dir
        );

        if (using_tmp_dir)
            free(dir);

        struct stat st = {0};

        if (stat(xstarter_dir, &st) == -1) {
            if (mkdir(xstarter_dir, 0700)) {
                set_err(ERR_XSTARTER_MKDIR_FAILED);
            } else {
                xstarter_dir_avail = True;
            }
        } else {
            xstarter_dir_avail = True;
        }
    }
}

void
set_err(int err_code)
{
    /* Only set the first encountered error */
    if (err == NO_ERR)
        err = err_code;
}

int
get_err()
{
    return err;
}

void
print_err()
{
    printf("Error code: %d\n", err);

    switch (err) {
        case NO_ERR:
            PRINT("No error");
            break;
        case ERR_UNKNOWN_APP_MODE:
            PRINT("Unknown application mode");
            break;
        case ERR_NO_XSTARTER_DIR:
            PRINT("No xstarter directory found");
            break;
        case ERR_DIRS_TOO_LONG:
            PRINT("List of directories is too long");
            break;
        case ERR_XSTARTER_MKDIR_FAILED:
            PRINT("Failed to create the ~/.xstarter.d directory");
        default:
            PRINT("Unknown error");
    }
}

void*
safe_malloc(size_t n, unsigned long line)
{
    void *p = malloc(n);

    if (! p) {
        fprintf(
            stderr,
            "[%s:%lu] Out of memory(%lu bytes)\n",
             __FILE__,
            line,
            (unsigned long) n
        );

        exit(EXIT_FAILURE);
    }

    return p;
}
