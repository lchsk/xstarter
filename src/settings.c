#include <stdlib.h>
#include "settings.h"
#include "utils_string.h"

static GKeyFile *conf_file = NULL;
static GQueue* paths = NULL;

static config_t* CONF = NULL;
static config_main_t* section_main = NULL;


// void list_of_strings(GQueue* list, char** data, int len)
// {
//     if (list == NULL) return;
// 
//     for (int i = 0; i < len; i++) {
//         g_queue_push_tail(list, data[i]);
//     }
// }

str_array_t*
get_string_list_from_config(GKeyFile const* conf_file, char const* section, char const* key)
{
    char* raw_dirs = g_key_file_get_string(conf_file, section, key, NULL);

    if (raw_dirs == NULL) return NULL;

    str_array_t* dirs = str_array_new((raw_dirs), ",");

    return dirs;
}

void load_config()
{
    GError *error = NULL;

    conf_file = g_key_file_new ();

    if (!g_key_file_load_from_file (conf_file, "/home/lchsk/projects/xstarter/bin/default.conf", G_KEY_FILE_NONE, &error))
    {
        g_error (error->message);
        // return -1;
    }

    section_main = malloc(sizeof(config_main_t));

    CONF = malloc(sizeof(config_t));
    *CONF = (config_t) {
        .section_main = section_main
    };

    section_main->dirs = get_string_list_from_config(conf_file, "Main", "dirs");
    section_main->no_gui = g_key_file_get_boolean (conf_file, "Main", "no_gui", false);
    section_main->executables_only = g_key_file_get_boolean (conf_file, "Main", "executables_only", true);

    g_key_file_free(conf_file);
}

void free_config()
{
    if (section_main) {
        str_array_free(section_main->dirs);
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
