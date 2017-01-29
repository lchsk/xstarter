#ifndef SETTINGS_H
#define SETTINGS_H

#include <glib.h>
#include "utils.h"
#include "utils_string.h"

/* Name of the configuration file that
   should be in a $HOME/.xstarter.d directory */

#define CONFIG_FILE "xstarter.conf"

#define PROGRAM_NAME "xstarter"
#define XSTARTER_VERSION "0.5.0"

typedef struct {
    bool verbose;
    char* config_path;
    bool force_cache_refresh;
} cmdline_t;

typedef struct {
    str_array_t *dirs;
    char *terminal;
    bool emacs_bindings;
    bool recent_apps_first;
    int min_query_len;
    bool allow_spaces;
    bool numeric_shortcuts;
    bool use_cache;
    bool auto_cache_refresh;
} config_main_t;

typedef struct {
    char *selected;
} config_colours_t;

typedef struct {
    config_main_t *section_main;
    config_colours_t *section_colours;
} config_t;

void load_config(cmdline_t *cmdline);
void free_config();

const config_t *config();
int read_cmdline(cmdline_t *cmdline, int argc, char **argv);
void free_cmdline(cmdline_t *cmdline);

#endif
