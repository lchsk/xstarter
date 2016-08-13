#ifndef TERM_H
#define TERM_H

/* Fix definition of KEY_BACKSPACE on FreeBSD */
#ifdef __FreeBSD__
#undef KEY_BACKSPACE
#define KEY_BACKSPACE (127)
#endif

#define KEY_ESCAPE (27)
#define KEY_RETURN (10)

#define XS_COLOR_BLUE (8)
#define XS_COLOR_RED (9)

#define XS_COLOR_PAIR_1 (1)
#define XS_COLOR_PAIR_2 (2)

#define MAX_INPUT_LENGTH (20)

void init_term_gui();
void free_term_gui();

void init_search();
void free_search();
void free_search_contents();
void printf_results();

void run_term();

void cache_loaded();

#endif
