#ifndef SCAN_H
#define SCAN_H

#include <glib.h>

#include "utils.h"
#include "settings.h"

void load_cache(cmdline_t *cmdline);
void free_cache();
void kill_scan();

GQueue *get_cache();
Boolean is_cache_ready();

#endif
