#ifndef SETTINGS_H
#define SETTINGS_H

#include <glib.h>
#include "utils.h"
#include "utils_string.h"

/* Name of the configuration file that
   should be in a $HOME/.xstarter.d directory */

#define CONFIG_FILE "xstarter.conf"

#define PROGRAM_NAME "xstarter"
#define XSTARTER_VERSION "0.3.0"

typedef struct {
    int mode;
    Boolean verbose;
    char* config_path;
} cmdline_t;

typedef struct {
    str_array_t *dirs;
    char *terminal;
    Boolean executables_only;
    Boolean emacs_bindings;
    Boolean recent_apps_first;
    int min_query_len;
    Boolean allow_spaces;
    Boolean numeric_shortcuts;
} config_main_t;

typedef struct {
    config_main_t *section_main;
} config_t;

void load_config(cmdline_t *cmdline);
void free_config();

const config_t *config();
int read_cmdline(cmdline_t *cmdline, int argc, char **argv);
void free_cmdline(cmdline_t *cmdline);

#endif
