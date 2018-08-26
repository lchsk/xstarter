#ifndef SCAN_H
#define SCAN_H

#include <glib.h>

#include "utils.h"
#include "settings.h"

void init_search(void);
void free_search(void);
bool search(const char *query, unsigned query_len);
GList *results;

void load_cache(cmdline_t *cmdline, bool extra_thread);
void print_cache_apps(void);
void free_cache(void);
void kill_scan(void);
bool is_cache_ready(void);

#endif
