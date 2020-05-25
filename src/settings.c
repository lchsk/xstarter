#include <getopt.h>
#include <stdlib.h>

#include "settings.h"
#include "utils.h"
#include "utils_string.h"

static GKeyFile *conf_file = NULL;

static config_t *CONF = NULL;
static config_main_t *section_main = NULL;
static config_colours_t *section_colours = NULL;

static void set_default_dirs(config_t *conf)
{
    conf->section_main->dirs = str_array_new(xs_strdup("$PATH"), ",");
}

static void set_default_terminal(config_t *conf)
{
    conf->section_main->terminal = xs_strdup("xterm");
}

static void set_default_emacs_bindings(config_t *conf)
{
    conf->section_main->emacs_bindings = true;
}

static void set_recent_apps_first(config_t *conf)
{
    conf->section_main->recent_apps_first = true;
}

static void set_min_query_len(config_t *conf)
{
    conf->section_main->min_query_len = 1;
}

static void set_allow_spaces(config_t *conf)
{
    conf->section_main->allow_spaces = true;
}

static void set_numeric_shortcuts(config_t *conf)
{
    conf->section_main->numeric_shortcuts = true;
}

static void set_use_cache(config_t *conf)
{
    conf->section_main->use_cache = true;
}

static void set_auto_cache_refresh(config_t *conf)
{
    conf->section_main->auto_cache_refresh = true;
}

static void set_default_colour_selected(config_t *conf)
{
    conf->section_colours->selected = xs_strdup("f44336");
}

static void set_default_configuration(config_t *conf)
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

    // Colours
    set_default_colour_selected(conf);
}

void load_config(cmdline_t *cmdline)
{
    GError *error = NULL;

    conf_file = g_key_file_new();

    char path[256];

    section_main = smalloc(sizeof(config_main_t));
    section_colours = smalloc(sizeof(config_colours_t));

    CONF = smalloc(sizeof(config_t));

    *CONF = (config_t){.section_main = section_main,
                       .section_colours = section_colours};

    if (cmdline->config_path) {
        if (snprintf(path, sizeof(path), "%s", cmdline->config_path) < 0) {
            return;
        }
    } else if (xstarter_dir_avail) {
        if (snprintf(path, sizeof(path), "%s/%s", xstarter_dir, CONFIG_FILE) <
            0) {
            return;
        }
    } else {
        set_err(ERR_NO_XSTARTER_DIR);
    }

    if (g_key_file_load_from_file(conf_file, path, G_KEY_FILE_NONE, NULL)) {
        // Read directories from config

        char *raw = g_key_file_get_string(conf_file, "Main", "dirs", NULL);

        char *raw_dirs = expand_tilde(raw, getenv("HOME"));
        free(raw);

        if (raw_dirs == NULL || strcmp(raw_dirs, "") == 0) {
            set_default_dirs(CONF);
        } else {
            section_main->dirs = str_array_new(xs_strdup(raw_dirs), ",");

            if (section_main->dirs == NULL)
                set_default_dirs(CONF);
        }

        /* Strip trailing spaces */
        str_array_strip(section_main->dirs);

        section_main->terminal =
            g_key_file_get_string(conf_file, "Main", "terminal", &error);

        if (error != NULL || strcmp(section_main->terminal, "") == 0) {
            set_default_terminal(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_colours->selected =
            g_key_file_get_string(conf_file, "Colours", "selected", &error);

        if (error != NULL || strcmp(section_colours->selected, "") == 0) {
            set_default_colour_selected(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_main->emacs_bindings =
            g_key_file_get_boolean(conf_file, "Main", "emacs_bindings", &error);

        if (error != NULL) {
            set_default_emacs_bindings(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_main->recent_apps_first = g_key_file_get_boolean(
            conf_file, "Main", "recent_apps_first", &error);

        if (error != NULL) {
            set_recent_apps_first(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_main->min_query_len =
            g_key_file_get_integer(conf_file, "Main", "min_query_len", &error);

        if (error != NULL) {
            set_min_query_len(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_main->allow_spaces =
            g_key_file_get_boolean(conf_file, "Main", "allow_spaces", &error);

        if (error != NULL) {
            set_allow_spaces(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_main->numeric_shortcuts = g_key_file_get_boolean(
            conf_file, "Main", "numeric_shortcuts", &error);

        if (error != NULL) {
            set_numeric_shortcuts(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_main->use_cache =
            g_key_file_get_boolean(conf_file, "Main", "use_cache", &error);

        if (error != NULL) {
            set_use_cache(CONF);
            g_error_free(error);
            error = NULL;
        }

        section_main->auto_cache_refresh = g_key_file_get_boolean(
            conf_file, "Main", "auto_cache_refresh", &error);

        if (error != NULL) {
            set_auto_cache_refresh(CONF);
            g_error_free(error);
            error = NULL;
        }

    } else {
        set_default_configuration(CONF);
    }

    g_key_file_free(conf_file);
}

void free_config()
{
    if (section_main) {
        str_array_free(section_main->dirs);
        free(section_main->terminal);
        free(section_main);
    }

    if (section_colours) {
        free(section_colours->selected);
        free(section_colours);
    }

    if (CONF) {
        free(CONF);
    }
}

const config_t *config()
{
    return CONF;
}

static void usage()
{
    printf("Usage: xstarter\n\n");
    printf("Optional arguments:\n");
    printf("\t-h\tShow help screen\n");
    printf("\t-v\tShow xstarter version\n");
    printf("\t-V\tBe verbose\n");
    printf("\t-e\tExecute application and detach it from terminal\n");
    printf("\t-r\tRefresh cache\n");
    printf("\t-c\tPath to the configuration file\n");
    printf("\t-P\tPrint a list of applications from cache\n");
}

static void print_version()
{
    printf("%s %s\n", PROGRAM_NAME, XSTARTER_VERSION);
}

int read_cmdline(cmdline_t *cmdline, int argc, char **argv)
{
    int c;
    int quit = false;

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

void free_cmdline(cmdline_t *cmdline)
{
    if (cmdline) {
        if (cmdline->config_path)
            free(cmdline->config_path);

        free(cmdline);
    }
}
