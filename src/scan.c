#define _GNU_SOURCE // strcasestr
#include <assert.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "list.h"
#include "scan.h"
#include "term.h"
#include "utils_string.h"

List *results;

static const int PATH = 1024;
static void recent_apps_on_top(void);
static List *get_cache(void);
static void *refresh_cache();

/* Paths of applications we will search to find what we want */
static List *search_paths = NULL;

/* Directories to traverse in order to find application paths */
static List *paths = NULL;
static bool cache_ready = false;

static pthread_t th_refresh_cache;
/* Set to true if cache refresh thread should be stopped */
static bool stop_traversing;

static CmdLine *cmdline;

void init_search(void)
{
    results = list_new(10);
    read_recently_open_list();
}

void free_search(void)
{
    if (results) {
        list_free(results);
    }
}

bool is_cache_ready(void)
{
    return cache_ready;
}

void free_cache(void)
{
    if (search_paths) {
        for (int i = 0; i < list_size(search_paths); i++) {
            char *t = list_get(search_paths, i);
            free(t);
        }
    }

    if (search_paths)
        list_free(search_paths);

    if (paths) {
        for (int i = 0; i < list_size(paths); i++) {
            char *t = list_get(paths, i);
            if (t)
                free(t);
        }
    }

    if (paths)
        list_free(paths);
}

void load_cache(CmdLine *cmdline_, bool extra_thread)
{
    cmdline = cmdline_;

    stop_traversing = false;

    if (!extra_thread) {
        refresh_cache();
        return;
    }

    int code = pthread_create(&th_refresh_cache, NULL, refresh_cache, NULL);

    if (code) {
        dump_debug(
            "Failed t create refresh_cache thread, pthread_create error code:");
        dump_debug_int(code);

        // Refresh cache in current thread
        refresh_cache();
    }
}

void kill_scan(void)
{
    if (!cache_ready) {
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
    const Config *conf = config_get();

    bool resp = true;

    if (query_len < conf->section_main->min_query_len) {
        return false;
    }

    if (query[0] == ' ')
        return false;

    List *cache = get_cache();

    int current_query_len = 1;
    StrArray *query_parts = NULL;

    if (conf->section_main->allow_spaces) {
        query_parts = str_array_new(xs_strdup(query), " ");

        if ((query_len > 0 && query[query_len - 1] == ' ') ||
            query_parts == NULL) {
            resp = false;
            goto free_query_parts;
        }

        current_query_len = query_parts->length;
    }

    list_free(results);
    results = list_new(10);

    for (int i = 0; i < list_size(cache); i++) {
        char *path = (char *)list_get(cache, i);
        bool found = true;

        if (current_query_len == 1) {
            char *name = xs_basename(path);

            if (strcasestr(name, query) != NULL) {
                list_append(results, path);
            }

            free(name);
        } else if (current_query_len > 1) {
            for (int i = 0; i < current_query_len; i++) {
                if (strcmp(query_parts->data[i], " ") == 0)
                    continue;

                char *name = (path);

                if (strstr(name, query_parts->data[i]) == NULL) {
                    found = false;
                    goto finish;
                }

            finish:
                free(name);

                if (!found)
                    break;
            }

            if (found) {
                list_append(results, path);
            }
        }
    }

    recent_apps_on_top();

free_query_parts:
    str_array_free(query_parts);

    return resp;
}

void print_cache_apps(void)
{
    // Print recently open apps
    for (int i = 0; i < recent_apps_cnt; i++) {
        printf("%s\n", recent_apps[i]);
    }

    List *cache = get_cache();

    // Print other apps
    for (int i = 0; i < list_size(cache); i++) {
        const char *path = list_get(search_paths, i);

        printf("%s\n", path);
    }
}

/*
Get apps that were recently started to the top of the list
*/
static void recent_apps_on_top(void)
{
    const Config *conf = config_get();

    if (!conf->section_main->recent_apps_first) {
        return;
    }

    List *temp = list_new(list_size(results));

    for (int i = 0; i < recent_apps_cnt; i++) {
        for (int j = 0; j < list_size(results); j++) {
            if (strcmp(list_get(results, j), recent_apps[i]) == 0) {
                char *el = list_get(results, j);
                list_append(temp, el);

                list_del(results, j);
                break;
            }
        }
    }

    for (int j = 0; j < list_size(results); j++) {
        char *r = list_get(results, j);
        list_append(temp, r);
    }

    list_free(results);
    results = temp;
}

static void listdir(char *name, int level)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name))) {
        return;
    }

    if (!(entry = readdir(dir))) {
        closedir(dir);
        return;
    }

    char buf[PATH];

    do {
        if (stop_traversing)
            break;

        if (entry->d_type == DT_DIR) {
            char path[PATH];
            int len =
                snprintf(path, sizeof(path) - 1, "%s/%s", name, entry->d_name);

            if (len < 0) {
                return;
            }

            path[len] = 0;

            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            listdir(path, level + 1);
        } else {
            struct stat sb;

            memset(buf, (unsigned char)' ', sizeof(buf));

            strcpy(buf, name);
            strcat(buf, "/");
            strcat(buf, entry->d_name);

            if (stat(buf, &sb) == 0 && sb.st_mode & S_IXUSR) {
                list_append(search_paths, xs_strdup(buf));
            }
        }
    } while ((entry = readdir(dir)) != NULL);

    closedir(dir);
}

static void cache_to_file()
{
    char path[1024];
    if (snprintf(path, sizeof(path), "%s/cache", xstarter_dir) < 0) {
        return;
    }

    FILE *file = fopen(path, "w");

    for (int i = 0; i < list_size(search_paths); i++) {
        char *t = list_get(search_paths, i);

        fprintf(file, "%s\n", t);
    }

    fclose(file);
}

static void read_cache_file()
{
    FILE *fptr;

    char path[1024];
    search_paths = list_new(10);
    if (snprintf(path, sizeof(path), "%s/cache", xstarter_dir) < 0) {
        return;
    }

    fptr = fopen(path, "r");

    if (!fptr) {
        return;
    }

    char line[1024];

    while (fgets(line, 1024, fptr)) {
        line[strcspn(line, "\n")] = 0;
        list_append(search_paths, xs_strdup(line));
    }

    fclose(fptr);
}

static bool cache_needs_refresh()
{
    /* If a user requested cache refresh from the cmdline */
    if (cmdline->force_cache_refresh)
        return true;

    const Config *conf = config_get();

    if (!conf->section_main->use_cache)
        return true;

    if (!conf->section_main->auto_cache_refresh)
        return false;

    /* If cache file does not exist then yes */

    struct stat cache_stat;
    time_t cache_time;

    char path[1024];
    if (snprintf(path, sizeof(path), "%s/cache", xstarter_dir) < 0) {
        return false;
    }

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

    for (int i = 0; i < list_size(paths); i++) {
        char *dir = list_get(paths, i);

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
    paths = list_new(10);

    StrArray *dirs = config_get()->section_main->dirs;

    for (int i = 0; i < dirs->length; i++) {
        if (dirs->data[i][0] == '$') {
            char const *var = getenv(++dirs->data[i]);

            if (var) {
                StrArray *var_paths = str_array_new(xs_strdup(var), ":");

                if (var_paths) {
                    for (int j = 0; j < var_paths->length; j++) {
                        if (var_paths->data[j]) {
                            list_append(paths, xs_strdup(var_paths->data[j]));
                        }
                    }
                }
                str_array_free(var_paths);
            }
        } else {
            list_append(paths, xs_strdup(dirs->data[i]));
        }
    }

    if (cache_needs_refresh()) {
        search_paths = list_new(10);

        for (int i = 0; i < list_size(paths); i++) {
            char *t = list_get(paths, i);

            listdir(t, 0);
        }

        const Config *conf = config_get();

        if (conf->section_main->use_cache)
            cache_to_file();
    } else
        read_cache_file();

    cache_ready = true;
    cache_loaded();

    return NULL;
}

static List *get_cache(void)
{
    return search_paths;
}
