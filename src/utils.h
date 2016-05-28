#ifndef UTILS_H
#define UTILS_H

#define True (1)
#define False (0)

/* MODES */

#define MODE_OPEN_IMMEDIATELY (1)
#define MODE_SAVE_TO_FILE (2)
#define MODE_RETURN_TERMINAL (3)

#define RECENT_APPS_REMEMBERED (3)

typedef struct {
    char path[1024];
} recently_open_t;

char xstarter_dir[1024];
int xstarter_dir_avail;

recently_open_t recent_apps[RECENT_APPS_REMEMBERED];
int recent_apps_cnt;

int running_from_term();
void app_to_open(char* path);
void open_app(const int mode);

/* int get_config_path(char* path); */

void dump_debug(const char* str);
void dump_debug_int(int d);

/* void get_working_dir(); */
void xstarter_directory();

void read_recently_open_list();

#endif
