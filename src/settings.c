#include <stdlib.h>
#include <unistd.h>
#include "settings.h"
#include "utils.h"
#include "utils_string.h"

static GKeyFile *conf_file = NULL;
static GQueue* paths = NULL;

static config_t* CONF = NULL;
static config_main_t* section_main = NULL;

static str_array_t*
get_string_list_from_config(
    GKeyFile* conf_file,
    char const* section,
    char const* key
)
{
    char* raw_dirs = g_key_file_get_string(
        conf_file,
        section,
        key,
        NULL
    );

    if (raw_dirs == NULL) return NULL;

    str_array_t* dirs = str_array_new((raw_dirs), ",");

    return dirs;
}

static void
set_default_configuration(config_main_t* section_main)
{
    section_main->dirs = str_array_new(strdup("$PATH"), ",");
    section_main->terminal = strdup("xterm");
    section_main->executables_only = 1;
}

void
load_config()
{
    GError* error = NULL;

    conf_file = g_key_file_new ();

    char home_dir[64];
    char path[256];

    section_main = malloc(sizeof(config_main_t));

    CONF = malloc(sizeof(config_t));
    *CONF = (config_t) {
        .section_main = section_main
    };

    if (! get_config_path(home_dir)) {
        snprintf(
            path,
            sizeof(path),
            "%s/%s",
            home_dir,
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
        &error
    )) {
        section_main->dirs = get_string_list_from_config(
            conf_file,
            "Main",
            "dirs"
       );

       section_main->terminal = g_key_file_get_string(
           conf_file,
           "Main",
           "terminal",
           &error
       );
       section_main->executables_only = g_key_file_get_boolean(
           conf_file,
           "Main",
           "executables_only",
           &error
       );
    } else {
        // TODO: handle non-existing config file
        set_default_configuration(section_main);
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

config_t* config()
{
    return CONF;
}

void
usage()
{
    printf("usage:");
}

void
read_cmdline(cmdline_t* cmdline, int argc, char** argv)
{
    int c;

    /* Default settings: */

    cmdline->mode = MODE_OPEN_IMMEDIATELY;
    cmdline->help = 0;

    while ((c = getopt(argc, argv, "tfh")) != -1) {
        switch(c) {
        case 't':
            cmdline->mode = MODE_RETURN_TERMINAL;
            break;
        case 'f':
            cmdline->mode = MODE_SAVE_TO_FILE;
            break;
        case 'h':
            cmdline->help = 1;
        }
    }
}
