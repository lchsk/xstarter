#ifndef SCAN_H
#define SCAN_H

#include <glib.h>

void load_cache();
void free_cache();

GQueue* get_cache();

#endif