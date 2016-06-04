#include <stdlib.h>
#include <unistd.h>
#include "settings.h"
#include "utils.h"
#include "utils_string.h"

static GKeyFile *conf_file = NULL;
static GQueue* paths = NULL;

static config_t* CONF = NULL;
static config_main_t* section_main = NULL;

static void
set_default_dirs(config_t* conf)
{
    conf->section_main->dirs = str_array_new(strdup("$PATH"), ",");
}

static void
set_default_terminal(config_t* conf)
{
    conf->section_main->terminal = strdup("xterm");
}

static void
set_default_executables_only(config_t* conf)
{
    conf->section_main->executables_only = True;
}

static void
set_default_emacs_bindings(config_t* conf)
{
    conf->section_main->emacs_bindings = True;
}

static void
set_recent_apps_first(config_t* conf)
{
    conf->section_main->recent_apps_first = True;
}

static void
set_default_configuration(config_t* conf)
{
    set_default_dirs(conf);
    set_default_terminal(conf);
    set_default_executables_only(conf);
    set_default_emacs_bindings(conf);
    set_recent_apps_first(conf);
}

void
load_config()
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

    if (xstarter_dir_avail) {
        snprintf(
            path,
            sizeof(path),
            "%s/%s",
            xstarter_dir,
            CONFIG_FILE
        );
    } else {
        /* TODO */
        // Critical error - quit...
    }

    if (g_key_file_load_from_file(
        conf_file,
        path,
        G_KEY_FILE_NONE,
        NULL
    )) {
        // Read directories from config

        char* raw_dirs = g_key_file_get_string(
            conf_file,
            "Main",
            "dirs",
            NULL
            );

        if (raw_dirs == NULL) {
            section_main->dirs = str_array_new(strdup("$PATH"), ",");
        } else {
            section_main->dirs = str_array_new(raw_dirs, ",");
        }

       section_main->terminal = g_key_file_get_string(
           conf_file,
           "Main",
           "terminal",
           &error
       );

       if (error != NULL) {
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

const config_t* config()
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
    printf("\t-t\tReturn terminal from the configuration\n");
    printf("\t\t(Intended for internal use)\n");
}

int
read_cmdline(cmdline_t* cmdline, int argc, char** argv)
{
    int c;
    int quit = False;

    /* Default settings: */

    cmdline->mode = MODE_OPEN_IMMEDIATELY;

    while ((c = getopt(argc, argv, "tfhv")) != -1) {
        switch(c) {
        case 't':
            cmdline->mode = MODE_RETURN_TERMINAL;
            break;
        case 'f':
            cmdline->mode = MODE_SAVE_TO_FILE;
            break;
        case 'v':
            printf("%s %s\n", PROGRAM_NAME, XSTARTER_VERSION);
            quit = True;
            break;
        case 'h':
        default:
            usage();
            quit = True;
        }
    }

    return quit;
}
