#ifndef XSTARTER_SCAN_H
#define XSTARTER_SCAN_H

#include "list.h"
#include "settings.h"
#include "utils.h"

void init_search(void);
void free_search(void);
bool search(const char *query, unsigned query_len);
extern List *results;

void load_cache(CmdLine *cmdline, bool extra_thread);
void print_cache_apps(void);
void free_cache(void);
void kill_scan(void);
bool is_cache_ready(void);

#endif
