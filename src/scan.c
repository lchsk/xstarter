#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glib.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

#include "scan.h"
#include "settings.h"
#include "utils_string.h"
#include "term.h"

int PATH = 1024;

/* Paths of applications we will search to find what we want */
static GQueue *search_paths = NULL;

/* Directories to traverse in order to find application paths */
static GQueue *paths = NULL;
static Boolean cache_ready = False;

static pthread_t th_refresh_cache;
/* Set to true if cache refresh thread should be stopped */
static Boolean stop_traversing;

static void
listdir(char *name, int level)
{
    DIR* dir;
    struct dirent *entry;

    if (! (dir = opendir(name))) {
        return;
    }

    if (! (entry = readdir(dir))) {
        return;
    }

    char buf[PATH];

    do {
        if (stop_traversing)
            break;

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
                g_queue_push_tail(search_paths, strdup(buf));
            }
        }
    } while ((entry = readdir(dir)) != NULL);

    closedir(dir);
}

static void
cache_to_file()
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/cache", xstarter_dir);

    FILE *file = fopen(path, "w");

    for (int i = 0; i < g_queue_get_length(search_paths); i++) {
        char *t = g_queue_peek_nth(search_paths, i);

        fprintf(file, "%s\n", t);
    }

    fclose(file);
}

static void
read_cache_file()
{
    FILE *fptr;

    char path[1024];
    search_paths = g_queue_new();
    snprintf(path, sizeof(path), "%s/cache", xstarter_dir);

    fptr = fopen(path, "r");

    if (fptr) {
        char line[1024];

        while (fgets(line, 1024, fptr)) {
            line[strcspn(line, "\n")] = 0;
            g_queue_push_tail(search_paths, strdup(line));
        }

        fclose(fptr);
    }
}

static Boolean
cache_needs_refresh()
{
    const config_t *conf = config();

    if (! conf->section_main->use_cache)
        return True;

    if (! conf->section_main->auto_cache_refresh)
        return False;

    /* If cache file does not exist then yes */

    struct stat cache_stat;
    time_t cache_time;

    char path[1024];
    snprintf(path, sizeof(path), "%s/cache", xstarter_dir);

    if (stat(path, &cache_stat) == -1) {
        /* Does not exist */
        return True;
    } else {
        /* If exists, get last modification time */
        cache_time = mktime(localtime(&(cache_stat.st_ctime)));
    }

    /* Compare last modification dates of cache and directories */

    struct stat dir_stat;
    time_t dir_time;

    for (int i = 0; i < g_queue_get_length(paths); i++) {
        char *dir = g_queue_peek_nth(paths, i);

        if (stat(dir, &dir_stat) == 0) {
            dir_time = mktime(localtime(&(dir_stat.st_ctime)));

            double diff = difftime(dir_time, cache_time);

            if (diff > 0) {
                /* A directory is more fresh that cache file - */
                /* Need to refresh */
                return True;
            }
        }
    }

    return False;
}

static void*
refresh_cache()
{
    paths = g_queue_new();

    str_array_t *dirs = config()->section_main->dirs;

    for (int i = 0; i < dirs->length; i++) {
        if (dirs->data[i][0] == '$') {
            char const *var = g_getenv(++dirs->data[i]);

            if (var != NULL) {
                str_array_t *var_paths = str_array_new(strdup(var), ":");

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

    if (cache_needs_refresh()) {
        search_paths = g_queue_new();

        char *path;

        for (int i = 0; i < g_queue_get_length(paths); i++) {
            char *t = g_queue_peek_nth(paths, i);

            listdir(t, 0);
        }

        const config_t *conf = config();

        if (conf->section_main->use_cache)
            cache_to_file();
    } else
        read_cache_file();

    cache_ready = True;
    cache_loaded();
}

void
load_cache()
{
    stop_traversing = False;

    int code = pthread_create(
        &th_refresh_cache,
        NULL,
        refresh_cache,
        NULL
    );

    assert(code == 0);
}

void
kill_scan()
{
    if (! cache_ready) {
        stop_traversing = True;
        pthread_join(th_refresh_cache, NULL);
    } else {
        pthread_detach(th_refresh_cache);
    }
}

void
free_cache()
{
    if (search_paths != NULL) {
        for (int i = 0; i < g_queue_get_length(search_paths); i++) {
            char *t = g_queue_peek_nth(search_paths, i);
            free(t);
        }
    }

    if (search_paths != NULL)
        g_queue_free(search_paths);

    if (paths != NULL) {
        for (int i = 0; i < g_queue_get_length(paths); i++) {
            char *t = g_queue_peek_nth(paths, i);
            free(t);
        }
    }

    if (paths != NULL)
        g_queue_free(paths);
}

GQueue
*get_cache()
{
    return search_paths;
}

Boolean
is_cache_ready()
{
	return cache_ready;
}
