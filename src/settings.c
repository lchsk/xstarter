#include <getopt.h>
#include <stdlib.h>

#include <inih/ini.h>

#include "settings.h"
#include "utils.h"
#include "utils_string.h"

#define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

static Config *_config = NULL;

static void set_default_dirs(Config *conf)
{
    conf->section_main->dirs = str_array_new(xs_strdup("$PATH"), ",");
}

static void set_default_terminal(Config *conf)
{
    conf->section_main->terminal = xs_strdup("xterm");
}

static void set_default_emacs_bindings(Config *conf)
{
    conf->section_main->emacs_bindings = true;
}

static void set_recent_apps_first(Config *conf)
{
    conf->section_main->recent_apps_first = true;
}

static void set_min_query_len(Config *conf)
{
    conf->section_main->min_query_len = 1;
}

static void set_allow_spaces(Config *conf)
{
    conf->section_main->allow_spaces = true;
}

static void set_numeric_shortcuts(Config *conf)
{
    conf->section_main->numeric_shortcuts = true;
}

static void set_use_cache(Config *conf)
{
    conf->section_main->use_cache = true;
}

static void set_auto_cache_refresh(Config *conf)
{
    conf->section_main->auto_cache_refresh = true;
}

static void set_default_color_selected(Config *conf)
{
    conf->section_colors->selected = xs_strdup("f44336");
}

static void set_default_configuration(Config *conf)
{
    // Main
    set_default_dirs(conf);
    set_default_terminal(conf);
    set_default_emacs_bindings(conf);
    set_recent_apps_first(conf);
    set_min_query_len(conf);
    set_allow_spaces(conf);
    set_numeric_shortcuts(conf);
    set_use_cache(conf);
    set_auto_cache_refresh(conf);

    // Colors
    set_default_color_selected(conf);
}

static int parse_bool(const char *value, bool *b)
{
    if (strcmp(value, "true") == 0) {
        *b = true;
        return 1;
    } else if (strcmp(value, "false") == 0) {
        *b = false;
        return 1;
    }

    return 0;
}

static int config_read(void *data, const char *section, const char *name,
                       const char *value)
{
    Config *conf = (Config *)data;

    if (MATCH("Main", "dirs")) {
        char *raw_dirs = expand_tilde(value, getenv("HOME"));

        if (raw_dirs == NULL || strcmp(raw_dirs, "") == 0) {
            set_default_dirs(conf);
        } else {
            conf->section_main->dirs = str_array_new(xs_strdup(raw_dirs), ",");

            if (conf->section_main->dirs == NULL) {
                set_default_dirs(conf);
            }
        }

        /* Strip trailing spaces */
        str_array_strip(conf->section_main->dirs);
    } else if (MATCH("Main", "terminal")) {
        conf->section_main->terminal = xs_strdup(value);

        if (strcmp(conf->section_main->terminal, "") == 0) {
            set_default_terminal(conf);
        }
    } else if (MATCH("Main", "emacs_bindings")) {
        if (!parse_bool(value, &conf->section_main->emacs_bindings)) {
            set_default_emacs_bindings(conf);
        }
    } else if (MATCH("Main", "recent_apps_first")) {
        if (!parse_bool(value, &conf->section_main->recent_apps_first)) {
            set_recent_apps_first(conf);
        }
    } else if (MATCH("Main", "min_query_len")) {
        conf->section_main->min_query_len = atoi(value);
    } else if (MATCH("Main", "numeric_shortcuts")) {
        if (!parse_bool(value, &conf->section_main->numeric_shortcuts)) {
            set_numeric_shortcuts(conf);
        }
    } else if (MATCH("Main", "use_cache")) {
        if (!parse_bool(value, &conf->section_main->use_cache)) {
            set_use_cache(conf);
        }
    } else if (MATCH("Main", "auto_cache_refresh")) {
        if (!parse_bool(value, &conf->section_main->auto_cache_refresh)) {
            set_auto_cache_refresh(conf);
        }
    } else if (MATCH("Main", "allow_spaces")) {
        if (!parse_bool(value, &conf->section_main->allow_spaces)) {
            set_allow_spaces(conf);
        }
    } else if (MATCH("Colours", "selected") || MATCH("Colors", "selected")) {
        conf->section_colors->selected = xs_strdup(value);
    } else {
        return 0;
    }

    return 1;
}

static Config *config_new()
{
    ConfigMain *section_main = smalloc(sizeof(ConfigMain));
    ConfigColors *section_colors = smalloc(sizeof(ConfigColors));

    Config *c = smalloc(sizeof(Config));

    *c = (Config){.section_main = section_main,
                  .section_colors = section_colors};

    return c;
}

Config *config_load(CmdLine *cmdline)
{
    char path[1024];
    Config *config = config_new();

    if (cmdline->config_path) {
        if (snprintf(path, sizeof(path), "%s", cmdline->config_path) < 0) {
            return config;
        }
    } else if (xstarter_dir_avail) {
        if (snprintf(path, sizeof(path), "%s/%s", xstarter_dir, CONFIG_FILE) <
            0) {
            return config;
        }
    } else {
        set_err(ERR_NO_XSTARTER_DIR);
    }

    if (ini_parse(path, config_read, config) < 0) {
        set_default_configuration(config);
    }

    _config = config;

    return config;
}

void config_free(Config *config)
{
    if (!config) {
        return;
    }

    if (config->section_main) {
        str_array_free(config->section_main->dirs);
        free(config->section_main->terminal);
        free(config->section_main);
    }

    if (config->section_colors) {
        free(config->section_colors->selected);
        free(config->section_colors);
    }

    free(config);
    config = NULL;
    _config = NULL;
}

const Config *config_get()
{
    return _config;
}

static void usage()
{
    printf("Usage: xstarter\n\n");
    printf("Optional arguments:\n");
    printf("  -h  Show help screen\n");
    printf("  -v  Show xstarter version\n");
    printf("  -V  Be verbose\n");
    printf("  -e  Execute application and detach it from terminal\n");
    printf("  -r  Refresh cache\n");
    printf("  -c  Path to the configuration file\n");
    printf("  -P  Print a list of applications from cache\n");
}

static void print_version()
{
    printf("%s %s\n", PROGRAM_NAME, XSTARTER_VERSION);
}

int cmdline_read(CmdLine *cmdline, int argc, char **argv)
{
    int c;
    bool quit = false;

    /* Default settings: */
    cmdline->config_path = NULL;
    cmdline->verbose = false;
    cmdline->force_cache_refresh = false;
    cmdline->print_list_of_cache_apps = false;

    while ((c = getopt(argc, argv, "hvre:VPc:")) != -1) {
        switch (c) {
        case 'c':
            cmdline->config_path = optarg;
            break;
        case 'e':
            open_app(optarg, "", APP_LAUNCH_MODE_GUI,
                     /* save_open_file */ true);
            quit = true;
            break;
        case 'v':
            print_version();
            quit = true;
            break;
        case 'V':
            cmdline->verbose = true;
            break;
        case 'r':
            cmdline->force_cache_refresh = true;
            break;
        case 'P':
            cmdline->print_list_of_cache_apps = true;
            break;
        case 'h':
        default:
            usage();
            quit = true;
        }
    }

    return quit;
}

void cmdline_free(CmdLine *cmdline)
{
    if (cmdline) {
        if (cmdline->config_path)
            free(cmdline->config_path);

        free(cmdline);
    }
}
