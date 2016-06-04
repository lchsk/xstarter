#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>

#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include "settings.h"
#include "term.h"
#include "scan.h"
#include "utils.h"

static GList* results = NULL;

static WINDOW* window = NULL;
static MENU* menu_list = NULL;
static ITEM** list_items = NULL;
static FORM* form = NULL;
static FIELD* field[2] = {NULL};

static char query[MAX_INPUT_LENGTH];
static int query_len = 0;

static char* choices[] = {
    (char*) NULL,
};

static int choices_cnt;
static int clear_items = False;
static int run_app = False;

static void
prepare_for_new_results() {

    if (menu_list) {
        unpost_menu(menu_list);

        // TODO: this causes a warning
        _nc_Disconnect_Items(menu_list);
    }

    if (list_items) {
        for (int i = 0; i < choices_cnt; i++) {
            free_item(list_items[i]);
        }

        free(list_items);
    }
}

static void
search(char* query)
{
    g_list_free(results);
    results = NULL;

    GQueue* cache = get_cache();

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        char* path = g_queue_peek_nth(cache, i);

        if (strstr(basename(path), query) != NULL) {
            results = g_list_prepend(results, path);
        }
    }

    /* Get apps that were recently started to the top of the list */

    int new_pos = 0;

    for (int i = 0; i < recent_apps_cnt; i++) {
        GList* to_remove = NULL;

        for (GList* l = results; l != NULL; l = l->next) {
            if (strcmp(l->data, recent_apps[i]) == 0) {
                results = g_list_insert(results, l->data, new_pos);

                to_remove = l;
                new_pos++;
            }
        }

        if (to_remove != NULL)
            results = g_list_delete_link(results, to_remove);
    }
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
        GList* l = g_list_nth(
            results,
            item_index(current_item(menu_list))
        );
        char* path = l->data;

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

static void
no_results()
{
    if (query_len > 0) {
        choices_cnt = 2;

        char* choices[] = {"No results, sorry", (char *) NULL};

        list_items = (ITEM**) calloc(choices_cnt, sizeof(ITEM*));

        list_items[0] = new_item(choices[0], (char*) NULL);
        list_items[1] = new_item((char*) NULL, (char*) NULL);
    } else {
        choices_cnt = 1;

        list_items = (ITEM**) calloc(choices_cnt, sizeof(ITEM*));
        list_items[0] = new_item((char*) NULL, (char*) NULL);
    }

    set_menu_items(menu_list, list_items);
    post_menu(menu_list);
    refresh();
}

static void
update_menu()
{
    choices_cnt = g_list_length(results);

    if (choices_cnt == 0) {
        no_results();
        update_info_bar(False);
        return;
    }

    list_items = (ITEM**) calloc(choices_cnt + 1, sizeof(ITEM *));

    for (int i = 0; i < choices_cnt; i++) {
        GList* l = g_list_nth(results, i);
        char* path = l->data;

        list_items[i] = new_item(basename(path), (char*) NULL);
    }

    list_items[choices_cnt] = new_item((char*) NULL, (char*) NULL);
    set_menu_items(menu_list, list_items);
    post_menu(menu_list);
    /* refresh(); */

    update_info_bar(True);
}

static void
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

    int recent_apps_valid = True;

    for (choices_cnt = 0; choices_cnt < RECENT_APPS_SHOWN; choices_cnt++) {
        if (recent_apps[choices_cnt] != NULL
            && strcmp(recent_apps[choices_cnt], "") != 0
            ) {
            for (int i = 0; i < strlen(recent_apps[choices_cnt]); i++) {
                if (! isprint(recent_apps[choices_cnt][i])) {
                    recent_apps_valid = False;
                    break;
                }
            }

            if (! recent_apps_valid)
                break;

            results = g_list_append(results, recent_apps[choices_cnt]);
        }
    }

    list_items = (ITEM**) calloc(choices_cnt, sizeof(ITEM*));
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

    /* set_menu_items(menu_list, list_items); */
    /* post_menu(menu_list); */
    /* wrefresh(window); */

    /* attron(COLOR_PAIR(2)); */
    mvprintw(0, 0, "This is xstarter. Start typing to search");
    mvprintw(LINES - 1, 0, "Arrow keys to navigate, <enter> to open, <esc> to quit");
    /* attroff(COLOR_PAIR(2)); */

    prepare_for_new_results();
    update_menu();
    /* refresh(); */
    wrefresh(window);
}

void
free_term_gui()
{
    if (menu_list) {
        unpost_menu(menu_list);
        free_menu(menu_list);
    }

    if (list_items) {
        for (int i = 0; i < choices_cnt; i++) {
            free_item(list_items[i]);
        }

        free(list_items);
    }

    unpost_form(form);

    if (form)
        free_form(form);

    free_field(field[0]);

    endwin();
}

void
init_search()
{
    results = NULL;
    read_recently_open_list();
}

void
free_search()
{
    if (results != NULL)
        g_list_free(results);
}

static void
set_app_to_run()
{
    ITEM* item = current_item(menu_list);

    if (item) {
        char* app_path = g_list_nth_data(results, item_index(item));

        if (app_path != NULL) {
            run_app = True;
            app_to_open(strdup(app_path));
        }
    }
}

static int
read_emacs_keys(const char* name)
{
    // TODO: Add C-v and M-v

    if (strcmp(name, "^N") == 0) {
        return KEY_DOWN;
    } else if (strcmp(name, "^P") == 0) {
        return KEY_UP;
    } else if (strcmp(name, "^C") == 0) {
        return KEY_ESCAPE;
    } else if (strcmp(name, "^W") == 0) {
        // TODO: Delete whole line
    } else if (strcmp(name, "^D") == 0) {
        return KEY_BACKSPACE;
    }
    // TODO C-v

    return -1;
}

void run_term()
{
    config_t* conf = config();

    move(1, 0);

    int c;

    while((c = getch()) != KEY_ESCAPE)
    {
        if (conf->section_main->emacs_bindings) {
            int key = read_emacs_keys(keyname(c));

            if (key != -1) {
                if (key == KEY_ESCAPE)
                    break;
                c = key;
            }
        }

        if (c == KEY_DOWN) {
                menu_driver(menu_list, REQ_DOWN_ITEM);
                update_info_bar(True);
        } else if (c == KEY_UP) {
                menu_driver(menu_list, REQ_UP_ITEM);
                update_info_bar(True);
        } else if (c == KEY_RETURN) {
                set_app_to_run(True);
        } else if (c == KEY_NPAGE) {
                menu_driver(menu_list, REQ_SCR_DPAGE);
                update_info_bar(True);
        } else if (c == KEY_PPAGE) {
                menu_driver(menu_list, REQ_SCR_UPAGE);
                update_info_bar(True);
        } else if (c == KEY_BACKSPACE) {
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
        } else if (isprint(c)){
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

        wrefresh(window);

        if (run_app == True) {
            break;
        }
    }
}
