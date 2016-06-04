#ifndef SETTINGS_H
#define SETTINGS_H

#include <glib.h>
#include "utils_string.h"

/* Name of the configuration file that
   should be in a HOME/.xstarter.d directory */

#define CONFIG_FILE "xstarter.conf"

#define PROGRAM_NAME "xstarter"
#define XSTARTER_VERSION "0.1.0"

typedef struct {
    int mode;
} cmdline_t;

typedef struct {
    str_array_t* dirs;
    gchar* terminal;
    int executables_only;
    int emacs_bindings;
    int recent_apps_first;
    int min_query_len;
} config_main_t;

typedef struct {
    config_main_t* section_main;
} config_t;

void load_config();
void free_config();
void usage();

const config_t* config();
int read_cmdline(cmdline_t* cmdline, int argc, char** argv);

#endif
