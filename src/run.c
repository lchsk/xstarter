#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LEN (1024)

int
main(int argc, char** argv)
{
    FILE* fp;
    char path[MAX_LEN];
    char command[MAX_LEN + 10];
    char terminal[32];

    printf("%d", argc);
    printf("%s", argv[0]);
    return 0;

    /* Get terminal name */

    /* Add checks for when the file doesnt exist */

    fp = popen("xstarter -t", "r");

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
        "%s -e ./xstarter -f",
        terminal
    );

    fp = popen(path, "r");
    pclose(fp);

    /* Read from pipe */

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int pipe = open("/tmp/xstarter", O_RDONLY, mode);

    read(pipe, path, MAX_LEN);
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
