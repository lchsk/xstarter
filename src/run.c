#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LEN (2048)

int
main(int argc, char** argv)
{
    FILE* fp;
    char path[MAX_LEN];
    char command[MAX_LEN + 10];
    char terminal[32];
    char xstarter_path[MAX_LEN];
    char xstarter_run[MAX_LEN];
    int xstarter_dir_found = 0;

    /* Get xstarter directory */

    ssize_t ret = readlink("/proc/self/exe", xstarter_path, sizeof(xstarter_path) - 1);

    if (ret != -1) {
        xstarter_dir_found = 1;
    }

    if (xstarter_dir_found) {
        dirname(xstarter_path);
    }

    struct stat sb;

    strcpy(xstarter_run, xstarter_path);
    strcat(xstarter_run, "/");
    strcat(xstarter_run, "xstarter");

    if (stat(xstarter_run, &sb) == 0 && sb.st_mode & S_IXUSR) {

    } else {
        printf("%s doesn't exist or is not executable", xstarter_run);
        exit(1);
    }

    /* Get terminal name */

    /* TODO: Add checks for when the file doesnt exist */

    char cmd[MAX_LEN];

    snprintf(
        cmd,
        sizeof(cmd),
        "%s -t",
        xstarter_run
    );

    fp = popen(cmd, "r");

    if (! fp) {
        printf("Failed to read terminal value");
        exit(1);
    }

    /* Fix this */

    while (fgets(terminal, sizeof(terminal), fp) != NULL);
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

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int pipe = open("/tmp/xstarter", O_RDONLY, mode);

    if (! read(pipe, path, MAX_LEN)) {
        /* Pipe is empty - quit */
        exit(0);
    }

    close(pipe);

    snprintf(
        command,
        500,
        "nohup %s &",
        path
    );

    /* Reset pipe */

    pipe = open("/tmp/xstarter", O_WRONLY | O_TRUNC);
    close(pipe);

    /* Run the application */

    system(command);

    return EXIT_SUCCESS;
}
