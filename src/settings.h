#ifndef XSTARTER_SETTINGS_H
#define XSTARTER_SETTINGS_H

#include "utils.h"
#include "utils_string.h"

/* Name of the configuration file that
   should be in a $HOME/.xstarter.d directory */

#define CONFIG_FILE "xstarter.conf"

#define PROGRAM_NAME "xstarter"
#define XSTARTER_VERSION "0.8.2"

typedef struct {
    bool verbose;
    char *config_path;
    bool force_cache_refresh;

    // Print a list of cache applications
    // Can be used by external programs
    bool print_list_of_cache_apps;
} CmdLine;

typedef struct {
    StrArray *dirs;
    char *terminal;
    bool emacs_bindings;
    bool recent_apps_first;
    int min_query_len;
    bool allow_spaces;
    bool numeric_shortcuts;
    bool use_cache;
    bool auto_cache_refresh;
} ConfigMain;

typedef struct {
    char *selected;
} ConfigColors;

typedef struct {
    ConfigMain *section_main;
    ConfigColors *section_colors;
} Config;

Config *config_load(CmdLine *cmdline);
void config_free(Config *config);
const Config *config_get();

int cmdline_read(CmdLine *cmdline, int argc, char **argv);
void cmdline_free(CmdLine *cmdline);

#endif
