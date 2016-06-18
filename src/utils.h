#ifndef UTILS_H
#define UTILS_H

#define True (1)
#define False (0)

#define Boolean short

/* MODES */

#define MODE_OPEN_IMMEDIATELY (1)
#define MODE_SAVE_TO_FILE (2)
#define MODE_RETURN_TERMINAL (3)

// TODO: Move those to the configuration file
#define RECENT_APPS_REMEMBERED (100)
#define RECENT_APPS_SHOWN (10)

#define ASCII_0 (48)
#define ASCII_1 (49)
#define ASCII_9 (57)

char xstarter_dir[1024];
int xstarter_dir_avail;

char recent_apps[100][1024];
int recent_apps_cnt;

int running_from_term();
void app_to_open(char* path);
void open_app(const int mode);

void dump_debug(const char* str);
void dump_debug_int(int d);
void dump_debug_char(const char);

void xstarter_directory();

void read_recently_open_list();

#endif
