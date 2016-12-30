#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>

#include <ncurses.h>

#include "settings.h"
#include "term.h"
#include "scan.h"
#include "utils.h"

static GList *results = NULL;

/* Names of applications (after basename) */
static GList *names = NULL;

static int MAX_Y = 15;

static char query[MAX_INPUT_LENGTH];
static int query_len = 0;

static char *choices[] = {
    (char*) NULL,
};

static int choices_cnt = 0;
static Boolean clear_items = False;
static Boolean run_app = False;
static Boolean results_not_found = False;

typedef struct {
    unsigned row_start;
    unsigned row_end;
} view_t;

static view_t view_search_bar = {
    .row_start = 0,
    .row_end = 0
};

static view_t view_menu = {
    .row_start = 3,
    .row_end = 12
};

static view_t view_info_bar = {
    .row_start = 13,
    .row_end = 14
};

typedef struct {
    char *items[1000];
    unsigned choices_cnt;
    unsigned offset;
    unsigned selected;
    char *error;
} items_list_t;

static items_list_t items_list;

static char *items[1000];

static void clean_line(int line_y)
{
    move(line_y, 0);
    clrtoeol();
}

static void erase_view(const view_t *view)
{
    for (int i = view->row_start; i <= view->row_end; i++)
        clean_line(i);
}

static void
clear_menu(Boolean clear)
{
    for (int i = 0; i < 1000; i++) {
        items_list.items[i] = NULL;
    }
}

static void
update_info_bar(void)
{
    erase_view(&view_info_bar);

    if (choices_cnt > 0) {
        if (items_list.selected >= choices_cnt) {
            items_list.selected = 0;
            items_list.offset = 0;
        }

        unsigned item = items_list.selected + items_list.offset;

        GList *l = g_list_nth(results, item);

        if (l) {
            char status[100];

            snprintf(status, 100, "Results: %d", choices_cnt);

            mvprintw(MAX_Y - 2, 0, status);
            mvprintw(MAX_Y - 1, 0, l->data);
        }
    }
}

static void draw_menu_item(unsigned i)
{
    int item_id = items_list.offset + i;

    if (items_list.items[item_id]) {
        char item[MAX_LIST_ITEM_LENGTH];

        unsigned shortcut = (i == 9) ? 0 : i + 1;

        char spaces[MAX_LIST_ITEM_LENGTH];

        unsigned item_len = strlen(items_list.items[item_id]);

        unsigned k = 0;

        for (unsigned j = item_len; j < MAX_LIST_ITEM_LENGTH; j++, k++)
            spaces[k] = ' ';

        spaces[k] = '\0';

        snprintf(
            item,
            MAX_LIST_ITEM_LENGTH,
            "%d %s%s",
            shortcut,
            items_list.items[item_id],
            spaces
        );

        mvprintw(i + 2, 0, item);
    }
}

void show_menu()
{
    for (int i = 0; i < 10; i++) {
        wmove(stdscr, i + 2, 0);

        wclrtoeol(stdscr);
    }

    if (choices_cnt) {
        for (unsigned i = 0; i < 10; i++) {
            if (i == items_list.selected)
                attron(COLOR_PAIR(1));

            draw_menu_item(i);

            if (i == items_list.selected)
                attroff(COLOR_PAIR(1));
        }
    } else {
        mvprintw(2, 0, "No results found, sorry");
    }
}

void move_down()
{
    if (items_list.selected + items_list.offset >= choices_cnt - 1)
        return;

    if (items_list.selected >= 9 && choices_cnt > 10) {
        items_list.offset++;
    } else if (items_list.selected < choices_cnt - 1)
        items_list.selected++;

    update_info_bar();
}

void move_up()
{
    if (items_list.selected + items_list.offset == 0)
        return;

    if (items_list.selected == 0 && items_list.offset > 0) {
        items_list.offset--;
    } else if (items_list.selected > 0)
        items_list.selected--;

    update_info_bar();
}

static void
prepare_for_new_results(Boolean clear)
{
    results_not_found = False;
    const config_t *conf = config();

    clear_menu(clear);

    choices_cnt = g_list_length(results);

    int cnt = choices_cnt > 1000 ? 1000 : choices_cnt;

    for (int i = 0; i < cnt; i++) {
        if (results_not_found) {
            if (query_len == 0) {
                items[i] = strdup("Start typing");
            } else {
                items[i] = strdup("No results");
            }
        } else {
            GList *l = g_list_nth(results, i);
            char *path = l->data;

            char *name = g_path_get_basename(path);
            names = g_list_prepend(names, name);

            if (conf->section_main->numeric_shortcuts) {
                if (i < 10) {
                    items_list.items[i] = name;
                }
                else
                    items_list.items[i] = name;
            } else {
                items_list.items[i] = name;
            }
        }
    }

    update_info_bar();
}

/* Get apps that were recently started to the top of the list */

static void
recent_apps_on_top(void)
{
    const config_t *conf = config();

    if (conf->section_main->recent_apps_first) {
        int new_pos = 0;

        for (int i = 0; i < recent_apps_cnt; i++) {
            GList* to_remove = NULL;

            for (GList *l = results; l != NULL; l = l->next) {
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

/* Return: */
    /* True: if search was successful and we need to update GUI */
    /* False: no need to update GUI */

static Boolean
search(char *const query)
{
    const config_t *conf = config();

    Boolean resp = True;
    results_not_found = True;

    if (query_len < conf->section_main->min_query_len) {
        return False;
    }

    if (query[0] == ' ')
        return False;

    GQueue *cache = get_cache();

    int current_query_len = 1;
    str_array_t *query_parts = NULL;

    if (conf->section_main->allow_spaces) {
        query_parts = str_array_new(strdup(query), " ");

        if (
            (query_len > 0 && query[query_len - 1] == ' ')
            || query_parts == NULL
        ) {
            resp = False;
            goto free_query_parts;
        }

        current_query_len = query_parts->length;
    }

    g_list_free(results);
    results = NULL;

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        char *path = g_queue_peek_nth(cache, i);
        Boolean found = True;

        if (current_query_len == 1) {
            char *name = g_path_get_basename(path);

            if (strcasestr(name, query) != NULL) {
                results = g_list_prepend(results, path);
            }

            free(name);
        } else if (current_query_len > 1) {
            for (int i = 0; i < current_query_len; i++) {
                if (strcmp(query_parts->data[i], " ") == 0)
                    continue;

                char *name = g_path_get_basename(
                    path
                );

                if (strstr(name, query_parts->data[i]) == NULL) {
                    found = False;
                    goto finish;
                }

            finish:
                free(name);

                if (! found)
                    break;
            }

            if (found)
                results = g_list_prepend(results, path);
        }
    }

    results_not_found = (g_list_length(results) > 0 ? False : True);

    recent_apps_on_top();

free_query_parts:
    str_array_free(query_parts);

    return resp;
}

static void
show_recent_apps(void)
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
init_term_gui(void)
{
    /* Fix ESC key */
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

    /* int max_rows; */
    /* int max_cols; */

    /* getmaxyx(stdscr, max_rows, max_cols); */

    show_recent_apps();
    prepare_for_new_results(False);

    /* Show recent apps */
    show_menu();

    erase_view(&view_search_bar);

    mvprintw(0, 0, "Loading paths...");

    refresh();

    /* Hide cursor */
    /* curs_set(0); */
}

void
free_term_gui(void)
{
}

void
init_search(void)
{
    results = NULL;
    read_recently_open_list();
}

void
free_search(void)
{
    if (results != NULL)
        g_list_free(results);
}

static void
open_app_later(const char *path)
{
    if (path != NULL) {
        run_app = True;
        app_to_open(strdup(path));
    }
}

static void
set_app_to_run(void)
{
    unsigned item = items_list.selected + items_list.offset;

    open_app_later(g_list_nth_data(results, item));
}

static void
open_by_shortcut(int key)
{
    const config_t *conf = config();

    if (conf->section_main->numeric_shortcuts) {
        if (key >= ASCII_1 && key <= ASCII_9) {
            unsigned item = items_list.offset + (key - ASCII_1);

            open_app_later(g_list_nth_data(results, item));
        } else if (key == ASCII_0) {
            open_app_later(g_list_nth_data(results, RECENT_APPS_SHOWN - 1));
        }
    }
}

static void
reset_query(void)
{
    erase_view(&view_search_bar);
    clear_menu(True);
    strcpy(query, "");
    query_len = 0;

    g_list_free(results);
    results = NULL;
    items_list.selected = 0;
    items_list.offset = 0;
    show_recent_apps();
    prepare_for_new_results(False);
}

static int
read_emacs_keys(const char *name)
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
    }

    return -1;
}

void cache_loaded(void)
{
    erase_view(&view_search_bar);
    mvprintw(0, 0, "");
    refresh();
}

void run_term(void)
{
    const config_t *conf = config();

    int c;

    while ((c = getch()) != KEY_ESCAPE) {
        if (conf->section_main->emacs_bindings) {
            int key = read_emacs_keys(keyname(c));

            if (key != -1) {
                if (key == KEY_ESCAPE)
                    break;
                c = key;
            }
        }

        open_by_shortcut(c);

        switch (c) {
        case KEY_DOWN:
            move_down();
            break;

        case KEY_UP:
            move_up();
            break;

        case KEY_RETURN:
            set_app_to_run();
            break;

        case KEY_DELETE:
        case KEY_BACKSPACE:
        case KEY_BACKSPACE_ALTERNATIVE:
            if (! query_len)
                continue;

            query_len--;

            if (query_len) {
                char *new_query = smalloc(query_len + 1);
                memcpy(new_query, query, query_len);
                new_query[query_len] = '\0';

                query[query_len] = 0;

                erase_view(&view_search_bar);
                mvprintw(0, 0, query);

                search(new_query);
                prepare_for_new_results(True);

                free(new_query);
            } else
                reset_query();

            break;

        default:

            if (query_len >= MAX_INPUT_LENGTH)
                continue;

            if (! isprint(c))
                continue;

            if (! is_cache_ready())
                continue;

            if (query_len == 0 && c == ' ') {
                reset_query();

                continue;
            } else if (query_len == 0) {
                clean_line(0);
            }

            query[query_len] = c;

            mvaddch(0, query_len, c);

            query_len++;
            char new_query[query_len];

            memcpy(new_query, query, query_len);
            new_query[query_len] = '\0';

            if (search(new_query))
                prepare_for_new_results(True);
        }

        show_menu();
        move(0, query_len);
        refresh();

        if (run_app)
            break;
    }
}
