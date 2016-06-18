#define _GNU_SOURCE

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

static int MAX_Y = 14;

static char query[MAX_INPUT_LENGTH];
static int query_len = 0;

static char* choices[] = {
    (char*) NULL,
};

static int choices_cnt = 0;
static Boolean clear_items = False;
static Boolean run_app = False;

static const char* digits[10] = {
    "(1)", "(2)", "(3)", "(4)", "(5)", "(6)", "(7)", "(8)", "(9)", "(0)"
};

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

/* Get apps that were recently started to the top of the list */

static void
recent_apps_on_top()
{
    const config_t* conf = config();

    if (conf->section_main->recent_apps_first) {
        int new_pos = 0;

        for (int i = 0; i < recent_apps_cnt; i++) {
            GList* to_remove = NULL;

            for (GList* l = results; l != NULL; l = l->next) {
                if (strcmp(l->data, recent_apps[i]) == 0) {
                    results = g_list_insert(results, l->data, new_pos);

                    to_remove = l;
                    new_pos++;
                    break;
                }
            }

            if (to_remove != NULL)
                results = g_list_delete_link(results, to_remove);
        }
    }
}

static void
search(char* query)
{
    const config_t* conf = config();

    if (query_len < conf->section_main->min_query_len) {
        return;
    }

    if (query[0] == ' ')
        return;

    GQueue* cache = get_cache();

    int current_query_len = 1;
    str_array_t* query_parts = NULL;

    if (conf->section_main->allow_spaces) {
        query_parts = str_array_new(strdup(query), " ");

        if (
            (query_len > 0 && query[query_len - 1] == ' ')
            || query_parts == NULL
        ) {
            goto free_query_parts;
        }

        current_query_len = query_parts->length;
    }

    g_list_free(results);
    results = NULL;

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        char* path = g_queue_peek_nth(cache, i);
        Boolean found = True;

        if (current_query_len == 1) {
            if (strcasestr(basename(path), query) != NULL) {
                results = g_list_prepend(results, path);
            }
        } else if (current_query_len > 1) {
            for (int i = 0; i < current_query_len; i++) {
                if (strcmp(query_parts->data[i], " ") == 0)
                    continue;

                if (strstr(basename(path), query_parts->data[i]) == NULL) {
                    found = False;
                    break;
                }
            }

            if (found)
                results = g_list_prepend(results, path);
        }
    }

    recent_apps_on_top();

free_query_parts:
    str_array_free(query_parts);
}

static void
clean_line(int line_y)
{
    move(line_y, 0);
    clrtoeol();
}

static void
clean_info_bar()
{
    clean_line(MAX_Y - 0);
    clean_line(MAX_Y - 1);
}

static void
update_info_bar(Boolean items_found)
{
    if (items_found) {
        GList* l = g_list_nth(
            results,
            item_index(current_item(menu_list))
        );

        char* path = l->data;

        clean_info_bar();

        char status[100];

        snprintf(status, 100, "Results: %d", choices_cnt);

        mvprintw(MAX_Y - 1, 0, status);
        mvprintw(MAX_Y, 0, path);
    } else {
        clean_info_bar();
    }

    refresh();
    wrefresh(window);
}

static void
no_results()
{
    if (query_len > 0) {
        choices_cnt = 2;

        char* choices[] = {"No results, sorry", (char*) NULL};

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

    const config_t* conf = config();

    list_items = (ITEM**) calloc(choices_cnt + 1, sizeof(ITEM *));

    for (int i = 0; i < choices_cnt; i++) {
        GList* l = g_list_nth(results, i);
        char* path = l->data;

        if (conf->section_main->numeric_shortcuts) {
            if (i < 10)
                list_items[i] = new_item(digits[i], basename(path));
            else
                list_items[i] = new_item(" ", basename(path));
        } else {
            list_items[i] = new_item(basename(path), (char*) NULL);
        }
    }

    list_items[choices_cnt] = new_item((char*) NULL, (char*) NULL);
    set_menu_items(menu_list, list_items);
    set_menu_format(menu_list, 10, 1);
    wrefresh(window);
    refresh();

    post_menu(menu_list);

    update_info_bar(True);
}

static void
remove_items()
{
    if (clear_items == False) return;

    for (int i = 0; i < choices_cnt; i++)
        free_item(list_items[i]);
}

static void
show_recent_apps()
{
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

    if (can_change_color()) {
        init_color(XS_COLOR_BLUE, 43, 180, 349);
        init_color(XS_COLOR_RED, 886, 27, 124);
        init_pair(XS_COLOR_PAIR_1, COLOR_WHITE, XS_COLOR_RED);
        init_pair(XS_COLOR_PAIR_2, COLOR_WHITE, XS_COLOR_BLUE);
    } else{
        init_pair(XS_COLOR_PAIR_1, COLOR_WHITE, COLOR_RED);
        init_pair(XS_COLOR_PAIR_2, COLOR_WHITE, COLOR_BLUE);
    }

    int max_rows;
    int max_cols;

    getmaxyx(stdscr, max_rows, max_cols);

    field[0] = new_field(
        1, // columns
        20, // width
        0, // pos y
        0, // pos x
        0,
        0
    );

    set_field_fore(field[0], COLOR_PAIR(XS_COLOR_PAIR_2));
    field[1] = NULL;

    form = new_form(field);
    post_form(form);
    refresh();

    show_recent_apps();

    list_items = (ITEM**) calloc(choices_cnt, sizeof(ITEM*));
    list_items[0] = new_item((char*) NULL, (char*) NULL);
    menu_list = new_menu((ITEM**) list_items);

    window = newwin(
        30, // rows
        max_cols, // cols
        2,
        0
    );

    keypad(window, TRUE);

    set_menu_win(menu_list, window);
    set_menu_mark(menu_list, "");

    box(window, 0, 0);
    wborder(window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

    mvprintw(0, 1, "This is xstarter. Start typing to search");

    set_menu_fore(menu_list, COLOR_PAIR(XS_COLOR_PAIR_1));

    prepare_for_new_results();
    update_menu();
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
open_app_later(const char* path)
{
    if (path != NULL) {
        run_app = True;
        app_to_open(strdup(path));
    }
}

static void
set_app_to_run()
{
    ITEM* item = current_item(menu_list);

    if (item) {
        open_app_later(g_list_nth_data(results, item_index(item)));
    }
}

static void
open_by_shortcut(int key)
{
    const config_t* conf = config();

    if (conf->section_main->numeric_shortcuts) {
        if (key >= ASCII_1 && key <= ASCII_9)
            open_app_later(g_list_nth_data(results, key - ASCII_1));
        else if (key == ASCII_0) {
            open_app_later(g_list_nth_data(results, 9));
        }
    }
}

static void
reset_query()
{
    strcpy(query, "");
    query_len = 0;
    form_driver(form, REQ_CLR_FIELD);
    form_driver(form, REQ_VALIDATION);

    g_list_free(results);
    results = NULL;

    prepare_for_new_results();
    show_recent_apps();
    update_menu();
    move(0, query_len);
}

static int
read_emacs_keys(const char* name)
{
    if (strcmp(name, "^N") == 0) {
        return KEY_DOWN;
    } else if (strcmp(name, "^P") == 0) {
        return KEY_UP;
    } else if (strcmp(name, "^C") == 0) {
        return KEY_ESCAPE;
    } else if (strcmp(name, "^W") == 0) {
        reset_query();
        return 0;
    } else if (strcmp(name, "^D") == 0) {
        return KEY_BACKSPACE;
    } else if (strcmp(name, "^V") == 0) {
        return KEY_NPAGE;
    } else if (strcmp(name, "M-6") == 0) {
        return KEY_PPAGE;
    }

    return -1;
}

void run_term()
{
    const config_t* conf = config();

    move(0, 0);

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

        open_by_shortcut(c);

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

                if (query_len == 0) {
                    reset_query();
                } else {
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
            }
        } else if (isprint(c)){
            if (query_len == 0) {
                clean_line(0);
            }

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
            move(0, query_len);
            wrefresh(window);

        if (run_app == True) {
            break;
        }
    }
}
