#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LEN (2048)
#define PIPE "/tmp/xstarter"

int
check_path(char *out, char *in)
{
    if (in[0] == '/') {
        /* Absolute path */

        strcpy(out, in);

        return 1;
    } else if (in[0] == '.' && in[1] == '/') {
        /* Path starting with ./ - append it to cwd */

        char cwd[MAX_LEN];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            char *xs = in;
            xs += 2;
            strcat(cwd, "/");
            strcat(cwd, xs);

            return 1;
        }
    }

    return 0;
}

/* Get path to xstarter using different methods */
/* Returns 1 if the path was found */
int
get_xstarter_path(int argc, char **argv, char *path)
{
    ssize_t ret;

    ret = readlink(
        "/proc/self/exe",
        path,
        MAX_LEN - 1
    );

    if (ret != -1) {
        return 1;
    }

    ret = readlink(
        "/proc/curproc/file",
        path,
        MAX_LEN - 1
    );

    if (ret != -1) {
        return 1;
    }

    if (argc >= 1) {
        if (check_path(path, argv[0]))
            return 1;
    }

    return 0;
}

int
main(int argc, char **argv)
{
    FILE *fp;
    char path[MAX_LEN];
    char command[MAX_LEN + 10];
    char terminal[32];
    char xstarter_path[MAX_LEN];
    char xstarter_run[MAX_LEN];
    int xstarter_dir_found = 0;
    int pipe;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    /* Write to pipe - xstarter should read it and reply to the pipe */

    pipe = open(
        PIPE,
        O_WRONLY | O_CREAT | O_TRUNC, mode
    );

    write(pipe, "-", 2);
    close(pipe);

    /* Get xstarter directory */

    xstarter_dir_found = get_xstarter_path(argc, argv, xstarter_path);

    if (xstarter_dir_found) {
        dirname(xstarter_path);
    }

    struct stat sb;

    strcpy(xstarter_run, xstarter_path);
    strcat(xstarter_run, "/");
    strcat(xstarter_run, "xstarter");

    if (! (stat(xstarter_run, &sb) == 0 && sb.st_mode & S_IXUSR)) {
        printf("%s doesn't exist or is not executable\n", xstarter_run);
        exit(1);
    }

    /* Get terminal name */

    char cmd[MAX_LEN];

    snprintf(
        cmd,
        sizeof(cmd),
        "%s -t",
        xstarter_run
    );

    fp = popen(cmd, "r");

    if (! fp) {
        printf("Failed to read terminal value\n");
        exit(1);
    }

    if (! fgets(terminal, sizeof(terminal), fp)) {
        printf("No terminal found\n");
        exit(1);
    }
    pclose(fp);

    /* Run xstarter on terminal */

    snprintf(
        path,
        MAX_LEN,
        "%s -e %s",
        terminal,
        xstarter_run
    );

    fp = popen(path, "r");
    pclose(fp);

    /* Read from pipe */

    pipe = open(PIPE, O_RDONLY, mode);

    if (! read(pipe, path, MAX_LEN)) {
        /* Pipe is empty - quit */
        exit(0);
    }

    close(pipe);

    snprintf(
        command,
        sizeof(command),
        "nohup %s 2> /dev/null &",
        path
    );

    /* Reset pipe */

    pipe = open(PIPE, O_WRONLY | O_TRUNC);
    close(pipe);

    /* Run the application */

    system(command);

    return EXIT_SUCCESS;
}
