#ifndef UTILS_H
#define UTILS_H

/* MODES */

#define MODE_OPEN_IMMEDIATELY (1)
#define MODE_SAVE_TO_FILE  (2)

int running_from_term();
void app_to_open(char* path);
void open_app(const int mode);

#endif
