#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <omp.h>
#include <glib.h>
#include <limits.h>
#include <string.h>

#include "scan.h"
#include "settings.h"
#include "utils_string.h"

static void refresh_cache();

int count = 0;
int PATH = 1024;

static GQueue* search_paths = NULL;
static GQueue* paths = NULL;

static void
listdir(char* name, int level)
{
    DIR* dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    char buf[PATH];

    do {
        if (entry->d_type == DT_DIR) {
            char path[PATH];
            int len = snprintf(
                path,
                sizeof(path) - 1,
                "%s/%s",
                name,
                entry->d_name
                );

            path[len] = 0;

            if (strcmp(entry->d_name, ".") == 0
                || strcmp(entry->d_name, "..") == 0)
                continue;

            listdir(path, level + 1);
        } else {
            struct stat sb;

            memset(buf, (unsigned char)' ', sizeof(buf));

            strcpy(buf, name);
            strcat(buf, "/");
            strcat(buf, entry->d_name);

            if (stat(buf, &sb) == 0 && sb.st_mode & S_IXUSR) {
                count++;
                g_queue_push_tail(search_paths,
                    strdup
                    (buf));
            }
        }
    } while ((entry = readdir(dir)) != NULL);

    closedir(dir);
}

static void
refresh_cache()
{
    paths = g_queue_new();

    str_array_t* dirs = config()->section_main->dirs;

    for (int i = 0; i < dirs->length; i++) {
        if (dirs->data[i][0] == '$') {
            char const* var = g_getenv(++dirs->data[i]);

            if (var != NULL) {
                str_array_t* var_paths = str_array_new(strdup(var), ":");

                if (var_paths != NULL) {
                    for (int j = 0; j < var_paths->length; j++) {
                        if (var_paths->data[j] != NULL) {
                            g_queue_push_tail(
                                paths,
                                strdup(var_paths->data[j])
                            );
                        }
                    }
                }
                str_array_free(var_paths);
            }
        } else {
            g_queue_push_tail(paths, strdup(dirs->data[i]));
        }
    }

    search_paths = g_queue_new();

    char* path;

    for (int i = 0; i < g_queue_get_length(paths); i++) {
        char* t = g_queue_peek_nth(paths, i);
        listdir(t, 0);
    }
}

void
load_cache()
{
    refresh_cache();
}

void free_cache()
{
    if (search_paths != NULL) {
        for (int i = 0; i < g_queue_get_length(search_paths); i++) {
            char* t = g_queue_peek_nth(search_paths, i);
            free(t);
        }
    }

    if (search_paths != NULL)
        g_queue_free(search_paths);

    if (paths != NULL) {
        for (int i = 0; i < g_queue_get_length(paths); i++) {
            char* t = g_queue_peek_nth(paths, i);
            free(t);
        }
    }

    if (paths != NULL)
        g_queue_free(paths);
}

GQueue* get_cache()
{
    return search_paths;
}
