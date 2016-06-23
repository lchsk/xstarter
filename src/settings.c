#include <stdlib.h>
#include <unistd.h>
#include "settings.h"
#include "utils.h"
#include "utils_string.h"

static GKeyFile *conf_file = NULL;
static GQueue *paths = NULL;

static config_t *CONF = NULL;
static config_main_t *section_main = NULL;

static void
set_default_dirs(config_t *conf)
{
    conf->section_main->dirs = str_array_new(strdup("$PATH"), ",");
}

static void
set_default_terminal(config_t *conf)
{
    conf->section_main->terminal = strdup("xterm");
}

static void
set_default_executables_only(config_t *conf)
{
    conf->section_main->executables_only = True;
}

static void
set_default_emacs_bindings(config_t *conf)
{
    conf->section_main->emacs_bindings = True;
}

static void
set_recent_apps_first(config_t *conf)
{
    conf->section_main->recent_apps_first = True;
}

static void
set_min_query_len(config_t *conf)
{
    conf->section_main->min_query_len = 1;
}

static void
set_allow_spaces(config_t *conf)
{
    conf->section_main->allow_spaces = True;
}

static void
set_numeric_shortcuts(config_t *conf)
{
    conf->section_main->numeric_shortcuts = True;
}

static void
set_default_configuration(config_t *conf)
{
    set_default_dirs(conf);
    set_default_terminal(conf);
    set_default_executables_only(conf);
    set_default_emacs_bindings(conf);
    set_recent_apps_first(conf);
    set_min_query_len(conf);
    set_allow_spaces(conf);
    set_numeric_shortcuts(conf);
}

void
load_config(cmdline_t *cmdline)
{
    GError* error = NULL;

    conf_file = g_key_file_new();

    char home_dir[64];
    char path[256];

    section_main = malloc(sizeof(config_main_t));

    CONF = malloc(sizeof(config_t));

    *CONF = (config_t) {
        .section_main = section_main
    };

    if (cmdline->config_path) {
        snprintf(path, sizeof(path), "%s", cmdline->config_path);
    } else if (xstarter_dir_avail) {
        snprintf(
            path,
            sizeof(path),
            "%s/%s",
            xstarter_dir,
            CONFIG_FILE
        );
    } else {
        set_err(ERR_NO_XSTARTER_DIR);
    }

    if (g_key_file_load_from_file(
        conf_file,
        path,
        G_KEY_FILE_NONE,
        NULL
    )) {
        // Read directories from config

        char *raw_dirs = g_key_file_get_string(
            conf_file,
            "Main",
            "dirs",
            NULL
        );

        if (raw_dirs == NULL || strcmp(raw_dirs, "") == 0) {
            section_main->dirs = str_array_new(strdup("$PATH"), ",");
        } else {
            section_main->dirs = str_array_new(raw_dirs, ",");

            if (section_main->dirs == NULL)
                section_main->dirs = str_array_new(strdup("$PATH"), ",");

        }

       section_main->terminal = g_key_file_get_string(
           conf_file,
           "Main",
           "terminal",
           &error
       );

       if (error != NULL || strcmp(section_main->terminal, "") == 0) {
           set_default_terminal(CONF);
           g_error_free(error);
           error = NULL;
       }

       section_main->executables_only = g_key_file_get_boolean(
           conf_file,
           "Main",
           "executables_only",
           &error
       );

       if (error != NULL) {
           set_default_executables_only(CONF);
           g_error_free(error);
           error = NULL;
       }

       section_main->emacs_bindings = g_key_file_get_boolean(
           conf_file,
           "Main",
           "emacs_bindings",
           &error
       );

       if (error != NULL) {
           set_default_emacs_bindings(CONF);
           g_error_free(error);
           error = NULL;
       }

       section_main->recent_apps_first = g_key_file_get_boolean(
           conf_file,
           "Main",
           "recent_apps_first",
           &error
       );

       if (error != NULL) {
           set_recent_apps_first(CONF);
           g_error_free(error);
           error = NULL;
       }

       section_main->min_query_len = g_key_file_get_integer(
           conf_file,
           "Main",
           "min_query_len",
           &error
       );

       if (error != NULL) {
           set_min_query_len(CONF);
           g_error_free(error);
           error = NULL;
       }

       section_main->allow_spaces = g_key_file_get_boolean(
           conf_file,
           "Main",
           "allow_spaces",
           &error
       );

       if (error != NULL) {
           set_allow_spaces(CONF);
           g_error_free(error);
           error = NULL;
       }

       section_main->numeric_shortcuts = g_key_file_get_boolean(
           conf_file,
           "Main",
           "numeric_shortcuts",
           &error
       );

       if (error != NULL) {
           set_numeric_shortcuts(CONF);
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

    if (CONF) {
        free(CONF);
    }
}

const
config_t *config()
{
    return CONF;
}

void
usage()
{
    printf("Usage: xstarter\n\n");
    printf("Optional arguments:\n");
    printf("\t-h\tShow help screen\n");
    printf("\t-v\tShow xstarter version\n");
    printf("\t-V\tBe verbose\n");
    printf("\t-c\tPath to the configuration file\n");
    printf("\t-t\tReturn terminal from the configuration\n");
}

int
read_cmdline(cmdline_t *cmdline, int argc, char **argv)
{
    int c;
    int quit = False;

    /* Default settings: */

    cmdline->mode = MODE_OPEN_APP;
    cmdline->config_path = NULL;
    cmdline->verbose = False;

    while ((c = getopt(argc, argv, "thvVc:")) != -1) {
        switch(c) {
        case 'c':
            cmdline->config_path = optarg;
            break;
        case 't':
            cmdline->mode = MODE_RETURN_TERMINAL;
            break;
        case 'v':
            printf("%s %s\n", PROGRAM_NAME, XSTARTER_VERSION);
            quit = True;
            break;
        case 'V':
            cmdline->verbose = True;
            break;
        case 'h':
        default:
            usage();
            quit = True;
        }
    }

    return quit;
}

void
free_cmdline(cmdline_t *cmdline)
{
    if (cmdline) {
        if (cmdline->config_path)
            free(cmdline->config_path);

        free(cmdline);
    }
}
