#include <stdlib.h>
#include <unistd.h>
#include "settings.h"
#include "utils.h"
#include "utils_string.h"

static GKeyFile *conf_file = NULL;
static GQueue* paths = NULL;

static config_t* CONF = NULL;
static config_main_t* section_main = NULL;

str_array_t*
get_string_list_from_config(GKeyFile* conf_file, char const* section, char const* key)
{
    char* raw_dirs = g_key_file_get_string(conf_file, section, key, NULL);

    if (raw_dirs == NULL) return NULL;

    str_array_t* dirs = str_array_new((raw_dirs), ",");

    return dirs;
}

void load_config()
{
    GError* error = NULL;

    conf_file = g_key_file_new ();

    if (
        ! g_key_file_load_from_file(
            conf_file,
            "/home/lchsk/projects/xstarter/bin/default.conf",
            G_KEY_FILE_NONE,
            &error
        )
    ){
        // TODO: handle non-existing config file
    }

    section_main = malloc(sizeof(config_main_t));

    CONF = malloc(sizeof(config_t));
    *CONF = (config_t) {
        .section_main = section_main
    };

    section_main->dirs = get_string_list_from_config(conf_file, "Main", "dirs");
    section_main->no_gui = g_key_file_get_boolean(
        conf_file,
        "Main",
        "no_gui",
        &error
    );
    section_main->executables_only = g_key_file_get_boolean(
        conf_file,
        "Main",
        "executables_only",
        &error
    );

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
			/* printf("rxvt-unicode"); */
			/* get user's terminal from config */
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
