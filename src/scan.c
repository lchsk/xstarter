#define _GNU_SOURCE // strcasestr
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
#include "utils_string.h"
#include "term.h"

static const int PATH = 1024;
static void recent_apps_on_top(void);
static GQueue *get_cache(void);
static void *refresh_cache();

/* Paths of applications we will search to find what we want */
static GQueue *search_paths = NULL;

/* Directories to traverse in order to find application paths */
static GQueue *paths = NULL;
static bool cache_ready = false;

static pthread_t th_refresh_cache;
/* Set to true if cache refresh thread should be stopped */
static bool stop_traversing;

static cmdline_t *cmdline;

void init_search(void)
{
    results = NULL;
    read_recently_open_list();
}

void free_search(void)
{
    if (results)
        g_list_free(results);
}

bool is_cache_ready(void)
{
    return cache_ready;
}

void free_cache(void)
{
    if (search_paths) {
        for (int i = 0; i < g_queue_get_length(search_paths); i++) {
            char *t = g_queue_peek_nth(search_paths, i);
            free(t);
        }
    }

    if (search_paths)
        g_queue_free(search_paths);

    if (paths) {
        for (int i = 0; i < g_queue_get_length(paths); i++) {
            char *t = g_queue_peek_nth(paths, i);
            free(t);
        }
    }

    if (paths)
        g_queue_free(paths);
}

void load_cache(cmdline_t *cmdline_, bool extra_thread)
{
    cmdline = cmdline_;

    stop_traversing = false;

    if (!extra_thread) {
        refresh_cache();
        return;
    }

    int code = pthread_create(
        &th_refresh_cache,
        NULL,
        refresh_cache,
        NULL
    );

    if (code) {
        dump_debug("Failed t create refresh_cache thread, pthread_create error code:");
        dump_debug_int(code);

        // Refresh cache in current thread
        refresh_cache();
    }
}

void kill_scan(void)
{
    if (! cache_ready) {
        stop_traversing = true;
        pthread_join(th_refresh_cache, NULL);
    } else {
        pthread_detach(th_refresh_cache);
    }
}

/*
Searches paths for a specific query

Returns:
    true: if search was successful and we need to update GUI
    false: no need to update GUI
*/
bool search(const char *query, unsigned query_len)
{
    const config_t *conf = config();

    bool resp = true;

    if (query_len < conf->section_main->min_query_len) {
        return false;
    }

    if (query[0] == ' ')
        return false;

    GQueue *cache = get_cache();

    int current_query_len = 1;
    str_array_t *query_parts = NULL;

    if (conf->section_main->allow_spaces) {
        query_parts = str_array_new(xs_strdup(query), " ");

        if ((query_len > 0 && query[query_len - 1] == ' ')
            || query_parts == NULL) {
            resp = false;
            goto free_query_parts;
        }

        current_query_len = query_parts->length;
    }

    g_list_free(results);
    results = NULL;

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        char *path = g_queue_peek_nth(cache, i);
        bool found = true;

        if (current_query_len == 1) {
            char *name = g_path_get_basename(path);

            if (strcasestr(name, query) != NULL) {
                results = g_list_prepend(results, path);
            }

            free(name);
        } else if (current_query_len > 1) {
            for (int i = 0; i < current_query_len; i++) {
                if (strcmp(query_parts->data[i], " ") == 0)
                    continue;

                char *name = g_path_get_basename(path);

                if (strstr(name, query_parts->data[i]) == NULL) {
                    found = false;
                    goto finish;
                }

            finish:
                free(name);

                if (! found)
                    break;
            }

            if (found)
                results = g_list_prepend(results, path);
        }
    }

    recent_apps_on_top();

free_query_parts:
    str_array_free(query_parts);

    return resp;
}

void print_cache_apps(void)
{
    GQueue *cache = get_cache();

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        const char *path = g_queue_peek_nth(search_paths, i);

        printf("%s\n", path);
    }
}

/*
Get apps that were recently started to the top of the list
*/
static void recent_apps_on_top(void)
{
    const config_t *conf = config();

    if (conf->section_main->recent_apps_first) {
        int new_pos = 0;

        for (int i = 0; i < recent_apps_cnt; i++) {
            GList* to_remove = NULL;

            for (GList *l = results; l != NULL; l = l->next) {
                if (strcmp(l->data, recent_apps[i]) == 0) {
                    results = g_list_insert(results, l->data, new_pos);

                    to_remove = l;
                    new_pos++;
                    break;
                }
            }

            if (to_remove != NULL)
                results = g_list_delete_link(results, to_remove);
        }
    }
}

static void listdir(char *name, int level)
{
    DIR *dir;
    struct dirent *entry;

    if (! (dir = opendir(name))) {
        return;
    }

    if (! (entry = readdir(dir))) {
        closedir(dir);
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
                g_queue_push_tail(search_paths, xs_strdup(buf));
            }
        }
    } while ((entry = readdir(dir)) != NULL);

    closedir(dir);
}

static void cache_to_file()
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

static void read_cache_file()
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
            g_queue_push_tail(search_paths, xs_strdup(line));
        }

        fclose(fptr);
    }
}

static bool cache_needs_refresh()
{
    /* If a user requested cache refresh from the cmdline */
    if (cmdline->force_cache_refresh)
        return true;

    const config_t *conf = config();

    if (! conf->section_main->use_cache)
        return true;

    if (! conf->section_main->auto_cache_refresh)
        return false;

    /* If cache file does not exist then yes */

    struct stat cache_stat;
    time_t cache_time;

    char path[1024];
    snprintf(path, sizeof(path), "%s/cache", xstarter_dir);

    if (stat(path, &cache_stat) == -1) {
        /* Does not exist */
        return true;
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
                return true;
            }
        }
    }

    return false;
}

static void *refresh_cache()
{
    paths = g_queue_new();

    str_array_t *dirs = config()->section_main->dirs;

    for (int i = 0; i < dirs->length; i++) {
        if (dirs->data[i][0] == '$') {
            char const *var = g_getenv(++dirs->data[i]);

            if (var) {
                str_array_t *var_paths = str_array_new(xs_strdup(var), ":");

                if (var_paths) {
                    for (int j = 0; j < var_paths->length; j++) {
                        if (var_paths->data[j]) {
                            g_queue_push_tail(
                                paths,
                                xs_strdup(var_paths->data[j])
                            );
                        }
                    }
                }
                str_array_free(var_paths);
            }
        } else {
            g_queue_push_tail(paths, xs_strdup(dirs->data[i]));
        }
    }

    if (cache_needs_refresh()) {
        search_paths = g_queue_new();

        for (int i = 0; i < g_queue_get_length(paths); i++) {
            char *t = g_queue_peek_nth(paths, i);

            listdir(t, 0);
        }

        const config_t *conf = config();

        if (conf->section_main->use_cache)
            cache_to_file();
    } else
        read_cache_file();

    cache_ready = true;
    cache_loaded();

    return NULL;
}

static GQueue *get_cache(void)
{
    return search_paths;
}
