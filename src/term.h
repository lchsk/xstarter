#ifndef TERM_H
#define TERM_H

// Seemed to help for ncurses 5.9
#define KEY_BACKSPACE_ALTERNATIVE (8)
#define KEY_DELETE (127)

#define KEY_ESCAPE (27)
#define KEY_RETURN (10)

#define XS_COLOR_BLUE (8)
#define XS_COLOR_RED (9)

#define XS_COLOR_PAIR_1 (1)
#define XS_COLOR_PAIR_2 (2)

#define MAX_INPUT_LENGTH (20)

/* In the list of applications */
const static unsigned MAX_LIST_ITEM_LENGTH = 24;

void init_term_gui();
void run_term();

void cache_loaded();

#endif
