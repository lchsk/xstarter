#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#define PRINTS(x) { printf("%s\n", x); }

#define RECENT_APPS_REMEMBERED (100)

#define ASCII_0 (48)
#define ASCII_1 (49)
#define ASCII_9 (57)

/* Errors */

typedef enum {
	NO_ERR,
	ERR_NO_XSTARTER_PATH,
	ERR_NO_XSTARTER_DIR,
	ERR_DIRS_TOO_LONG,
	ERR_XSTARTER_MKDIR_FAILED,
	ERR_FORK_FAILED,
	ERR_SETSID_FAILED,
	ERR_CHDIR_FAILED,
	ERR_DUMP_DEBUG_FAILED,
} error_code_t;

typedef enum {
	APP_LAUNCH_MODE_GUI,
	APP_LAUNCH_MODE_TERM
} app_launch_mode_t;

#define MAX_LEN (2048)

void open_app(const char *path, const char *query, app_launch_mode_t mode,
			  bool save_open_file);

/* Error code */
int err;

char xstarter_dir[1024];
int xstarter_dir_avail;

char recent_apps[100][1024];
int recent_apps_cnt;

char exec_term[32];

typedef struct {
	double r, g, b;
} colour_t;

void get_rgb(colour_t *dest, char *src);

void app_to_open(char *path);

void dump_debug(const char *str);
void dump_debug_ptr(const char *str);
void dump_debug_int(int d);
void dump_debug_char(const char);

void xstarter_directory();
void read_recently_open_list();

bool in_terminal();
void open_itself(int argc, char **argv);

void set_err(int err_code);
int get_err();
void print_err();

void *safe_malloc(size_t n, unsigned long line);
#define smalloc(n) safe_malloc(n, __LINE__);

#endif
