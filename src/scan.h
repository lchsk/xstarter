#ifndef SCAN_H
#define SCAN_H

#include <glib.h>

#include "utils.h"
#include "settings.h"

void init_search(void);
void free_search(void);
Boolean search(const char *query);
GList *results;
Boolean results_not_found;
int query_len;

void load_cache(cmdline_t *cmdline);
void free_cache(void);
void kill_scan(void);
Boolean is_cache_ready(void);

static void recent_apps_on_top(void);
static GQueue *get_cache(void);
static void *refresh_cache();

#endif
