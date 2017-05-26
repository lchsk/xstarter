#define _GNU_SOURCE
#include <unistd.h> // execve, readlink
#include <sys/stat.h> // umask
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "utils.h"

static void record_open_file(const char *path);
static bool check_path(char *out, char *in);
static bool get_xstarter_path(int argc, char **argv, char *path);

static const char *error_messages[] = {
	"No error, all's fine",
	"No xstarter path found",
	"No xstarter directory found",
	"List of directories is too long",
	"Failed to create the ~/.xstarter.d directory",
	"Fork failed",
	"setsid() failed",
	"chdir() failed"
};

void get_rgb(colour_t *dest, char *src)
{
    if (strlen(src) && src[0] == ';')
        src++;

    int c = (int)strtol(src, NULL, 16);

    dest->r = ((c >> 16) & 0xff) / 255.0 * 1000;
    dest->g = ((c >> 8) & 0xff) / 255.0 * 1000;
    dest->b = (c & 0xff) / 255.0 * 1000;
}

void open_app(char *path)
{
    if (! path) return;

    char path_cpy[1024];
    strcpy(path_cpy, path);

    record_open_file(path_cpy);

    pid_t pid;

    switch (pid = fork()) {
    case -1:
        dump_debug("fork() failed");
        dump_debug_int(errno);

        set_err(ERR_FORK_FAILED);

        break;

    case 0: // Child
        /* Change the file mode mask */
        umask(0);

        if (setsid() < 0) {
            dump_debug("setsid() failed");
            dump_debug_int(errno);

            set_err(ERR_SETSID_FAILED);
        }

        if (chdir("/") < 0) {
            dump_debug("chdir() failed");
            dump_debug_int(errno);

            set_err(ERR_CHDIR_FAILED);
        }

        /* Redirect standard files to /dev/null */
        freopen("/dev/null", "r", stdin);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        extern char** environ;
        char *argv[] = {path_cpy, NULL};

        execve(argv[0], &argv[0], environ);
    }
}

static void record_open_file(const char *path)
{
    if (xstarter_dir_avail) {
        char recently_f[1024];

        snprintf(recently_f, sizeof(recently_f), "%s/recent", xstarter_dir);

        FILE *file = fopen(recently_f, "w");

        if (file) {
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

void read_recently_open_list()
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

void dump_debug(const char *str)
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

void dump_debug_ptr(const char *str)
{
    char debug[1024];

    snprintf(
        debug,
        1024,
        "echo %p>> ~/debug_xstarter",
        &str
    );

    system(debug);
}

void dump_debug_char(const char c)
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

void dump_debug_int(int d)
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

bool in_terminal()
{
    bool xstarter_run = getenv("XSTARTER") ? true : false;

    return xstarter_run || isatty(STDOUT_FILENO);
}

void open_itself(int argc, char **argv)
{
    pid_t pid = fork();

    if (pid < 0) {
        dump_debug("Fork failed");
        exit(1);
    } else if (pid > 0) {
        exit(0);
    } else {
        char xstarter_path[MAX_LEN];

        if (! get_xstarter_path(argc, argv, xstarter_path)) {
            dump_debug("xstarter path not found, will try $PATH");
            dump_debug_int(errno);

            set_err(ERR_NO_XSTARTER_PATH);

            strcpy(xstarter_path, "xstarter");
        }

        umask(0);

        if (setsid() < 0) {
            dump_debug("setsid failed");
            dump_debug_int(errno);

            set_err(ERR_SETSID_FAILED);
        }

        if (chdir("/") < 0) {
            dump_debug("chdir failed");
            dump_debug_int(errno);

            set_err(ERR_CHDIR_FAILED);
        }

        /* Redirect standard files to /dev/null */
        freopen("/dev/null", "r", stdin);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        extern char **environ;

        int env_cnt = 0;

        for (char *it = environ[0]; *it; it++, env_cnt++);

        char **newenvp = malloc((env_cnt + 2) * sizeof(char*));

        for (int i = 0; i < env_cnt - 2; i++) {
            newenvp[i] = environ[i];
        }

        newenvp[env_cnt - 2] = "XSTARTER=1";
        newenvp[env_cnt - 1] = NULL;

        char *term_argv[] = {exec_term, "-e", xstarter_path, NULL};

        execvpe(term_argv[0], &term_argv[0], newenvp);
    }
}

void xstarter_directory()
{
    xstarter_dir_avail = true;
    bool using_tmp_dir = false;

    char *dir = NULL;

    if ((dir = (getenv("HOME"))) == NULL) {
        struct passwd *pw = getpwuid(getuid());
        dir = pw->pw_dir;
    }

    if (! dir) {
        using_tmp_dir = true;
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
                xstarter_dir_avail = true;
            }
        } else {
            xstarter_dir_avail = true;
        }
    }
}

void set_err(int err_code)
{
    /* Only set the first encountered error */
    if (err == NO_ERR)
        err = err_code;
}

int get_err()
{
    return err;
}

void print_err()
{
    printf("Error code: %d\n", err);

    ssize_t cnt = sizeof(error_messages) / sizeof(char *);

    if (err < cnt) {
        PRINTS(error_messages[err]);
    } else {
        PRINTS("Unknown error");
    }
}

void *safe_malloc(size_t n, unsigned long line)
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

static bool check_path(char *out, char *in)
{
    if (in[0] == '/') {
        /* Absolute path */

        strcpy(out, in);

        return true;
    } else if (in[0] == '.' && in[1] == '/') {
        /* Path starting with ./ - append it to cwd */

        char cwd[MAX_LEN];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            char *xs = in;
            xs += 2;
            strcat(cwd, "/");
            strcat(cwd, xs);

            return true;
        }
    }

    return false;
}

/*
Get path to xstarter using different methods

Returns true if the path was found, false otherwise
*/
static bool get_xstarter_path(int argc, char **argv, char *path)
{
    if (argc >= 1)
        return check_path(path, argv[0]);

    ssize_t len;

    len = readlink("/proc/self/exe", path, MAX_LEN - 1);

    if (len != -1) {
        path[len] = '\0';

        return true;
    }

    len = readlink("/proc/curproc/file", path, MAX_LEN - 1);

    if (len != -1) {
        path[len] = '\0';

        return true;
    }

    return false;
}
