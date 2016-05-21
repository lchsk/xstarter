#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include "term.h"
#include "scan.h"
#include "utils.h"

static GQueue* results = NULL;

static WINDOW* window;
static MENU* menu_list;
static ITEM** list_items;
static FORM  *form;
static FIELD* field[2];

static char query[MAX_INPUT_LENGTH];
static int query_len = 0;

static char* choices[] = {
    (char*) NULL,
};

static int choices_cnt;
static int clear_items = False;
static int run_app = False;

/* TODO: Move it somewhere else or delete */
void
printf_results()
{
    for (int i = 0; i < g_queue_get_length(results); i++) {
        char* t = g_queue_peek_nth(results, i);
        printf("%s\n", t);
    }
}

void
prepare_for_new_results() {
    unpost_menu(menu_list);

    // TODO: this causes a warning
    _nc_Disconnect_Items(menu_list);

    for (int i = 0; i < choices_cnt; i++) {
        free_item(list_items[i]);
    }

    free(list_items);
}

void
search(char* query)
{
    g_queue_clear(results);

    GQueue* cache = get_cache();

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        char* t = g_queue_peek_nth(cache, i);

        if (strstr(t, query) != NULL) {
            g_queue_push_tail(results, (t));
        }
    }
}

/* TODO: setting items here probably not needed */
void
set_items()
{
    clear_items = True;
    unpost_menu(menu_list);

    for(int i = 0; i < choices_cnt; ++i) {
        free_item(list_items[i]);
    }

    choices_cnt = g_queue_get_length(results) + 1;
    list_items = (ITEM**) calloc(choices_cnt, sizeof(ITEM*));

    for (int i = 0; i < g_queue_get_length(results); i++) {
        char* path = g_queue_peek_nth(results, i);

        list_items[i] = new_item(strdup(path), "");
    }

    set_menu_items(menu_list, list_items);
    post_menu(menu_list);
}

/* TODO: delete */
void
create_menu()
{
}

static void
clean_line(int line_y)
{
    move(line_y, 0);
    clrtoeol();
}

static void
update_info_bar(int items_found)
{
    if (items_found) {
        char* path = g_queue_peek_nth(
            results,
            item_index(
                current_item(menu_list)
            )
        );

        clean_line(LINES - 1);
        clean_line(LINES - 2);

        char status[100];

        snprintf(status, 100, "Results: %d", choices_cnt);

        mvprintw(LINES - 2, 0, status);
        mvprintw(LINES - 1, 0, path);
    } else {
        clean_line(LINES - 1);
        clean_line(LINES - 2);
    }

    refresh();
}

void
update_menu()
{
    choices_cnt = g_queue_get_length(results);

    if (choices_cnt == 0) {
        no_results();
        update_info_bar(False);
        return;
    }

    list_items = (ITEM**) calloc(choices_cnt + 1, sizeof(ITEM *));

    for (int i = 0; i < choices_cnt; i++) {
        char* path = g_queue_peek_nth(results, i);

        list_items[i] = new_item(basename(path), (char*) NULL);
    }

    list_items[choices_cnt] = new_item((char*) NULL, (char*) NULL);
    set_menu_items(menu_list, list_items);
    post_menu(menu_list);
    /* refresh(); */

    update_info_bar(True);
}
void
no_results()
{
    list_items = (ITEM**) calloc(2, sizeof(ITEM*));

    char* choices[] = {"No results, sorry", (char *) NULL};

    list_items[0] = new_item(choices[0], (char*) NULL);
    list_items[1] = new_item((char*) NULL, (char*) NULL);

    set_menu_items(menu_list, list_items);
    post_menu(menu_list);
    refresh();
    choices_cnt = 2;
}

void
remove_items()
{
    if (clear_items == False) return;

    for (int i = 0; i < choices_cnt; i++)
        free_item(list_items[i]);
}

void
init_term_gui()
{
    set_escdelay(25);

    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    /* init_color(10, 200, 20, 550); */
    /* if (can_change_color()) { */


    /* } else{ */

    /* } */

    /* init_color(COLOR_RED, 200, 320, 600); */
    init_pair(1, COLOR_RED, COLOR_RED);
    init_pair(2, COLOR_GREEN, COLOR_RED);

    int max_rows;
    int max_cols;

    getmaxyx(stdscr, max_rows, max_cols);

    field[0] = new_field(
        1, // columns
        20, // width
        1, // pos y
        0, // pos x
        0,
        0
    );

    field[1] = NULL;

    form = new_form(field);
    post_form(form);
    refresh();

    list_items = (ITEM**) calloc(1, sizeof(ITEM*));
    list_items[0] = new_item((char*) NULL, (char*) NULL);
    menu_list = new_menu((ITEM**) list_items);

    window = newwin(
        30, // rows
        max_cols, // cols
        3,
        0
    );

    keypad(window, TRUE);

    set_menu_win(menu_list, window);
    set_menu_format(menu_list, 6, 1);

    set_menu_mark(menu_list, " ");

    box(window, 0, 0);
    wborder(window, ' ', ' ', ' ',' ',' ',' ',' ',' ');

    post_menu(menu_list);
    choices_cnt = 2;
    /* wrefresh(window); */

    /* attron(COLOR_PAIR(2)); */
    mvprintw(0, 0, "This is xstarter. Start typing to search");
    mvprintw(LINES - 1, 0, "Arrow keys to navigate, <enter> to open, <esc> to quit");
    /* attroff(COLOR_PAIR(2)); */
    refresh();
}

void
free_term_gui()
{
    unpost_menu(menu_list);
    free_menu(menu_list);

    for (int i = 0; i < choices_cnt; i++) {
        free_item(list_items[i]);
    }

    free(list_items);

    unpost_form(form);
    free_form(form);
    free_field(field[0]);

    endwin();
}

void
init_search()
{
    results = g_queue_new();
}

void
free_search()
{
    if (results != NULL)
        g_queue_free(results);
}

static void
set_app_to_run()
{
    ITEM* item = current_item(menu_list);

    if (item) {
        char* app_path = g_queue_peek_nth(results, item_index(item));
        run_app = True;
        app_to_open(strdup(app_path));
    }
}

void run_term()
{
    move(1, 0);

    int c;

    while((c = getch()) != KEY_ESCAPE)
    {
        switch(c)
        {
            case KEY_DOWN:
                menu_driver(menu_list, REQ_DOWN_ITEM);
                update_info_bar(True);
                break;
            case KEY_UP:
                menu_driver(menu_list, REQ_UP_ITEM);
                update_info_bar(True);
                break;
            case KEY_RETURN:
                set_app_to_run(True);
                break;
            case KEY_NPAGE:
                menu_driver(menu_list, REQ_SCR_DPAGE);
                update_info_bar(True);
                break;
            case KEY_PPAGE:
                menu_driver(menu_list, REQ_SCR_UPAGE);
                update_info_bar(True);
                break;
            case KEY_BACKSPACE:
                if (query_len >= 1) {
                    query_len--;

                    form_driver(form, REQ_DEL_PREV);
                    form_driver(form, REQ_VALIDATION);

                    snprintf(
                        query,
                        MAX_INPUT_LENGTH,
                        "%s",
                        field_buffer(field[0], 0)
                    );

                    char* new_query = malloc(query_len + 1);
                    memcpy(new_query, query, query_len);
                    new_query[query_len] = '\0';

                    prepare_for_new_results();

                    search(new_query);

                    update_menu();

                    free(new_query);
                }

                break;
            default:
                // TODO: only letters/digits
                if ((int) c < 256) {
                    form_driver(form, c);
                    form_driver(form, REQ_VALIDATION);

                    snprintf(
                        query,
                        MAX_INPUT_LENGTH,
                        "%s",
                        field_buffer(field[0], 0)
					);

                    query_len++;
                    char new_query[query_len];

                    memcpy(new_query, query, query_len);
					new_query[query_len] = '\0';

                    prepare_for_new_results();

                    search(new_query);
                    update_menu();
                }
        }

        wrefresh(window);

        if (run_app == True) {
            break;
        }
    }
}
