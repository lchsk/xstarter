#ifndef TERM_H
#define TERM_H

#define KEY_ESCAPE 27
#define KEY_RETURN 10

#define MAX_INPUT_LENGTH 20

void init_term_gui();
void free_term_gui();

void init_search();
void free_search();
void free_search_contents();
void printf_results();

void run_term();

void create_menu();
void update_menu();
void no_results();

#endif
