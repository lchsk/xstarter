#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <unistd.h> // execve
#include <sys/stat.h> // umask
#include <errno.h>

#include <ncurses.h>

#include "settings.h"
#include "term.h"
#include "scan.h"
#include "utils.h"

/* Names of applications (after basename) */
static GList *names = NULL;

static int MAX_Y = 15;

static char query[MAX_INPUT_LENGTH];

static char *choices[] = {
    (char*) NULL,
};

static int choices_cnt = 0;
static Boolean clear_items = False;
static Boolean run_app = False;

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

static void clear_menu(Boolean clear)
{
    for (int i = 0; i < 1000; i++) {
        items_list.items[i] = NULL;
    }
}

static void update_info_bar(void)
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

static void prepare_for_new_results(Boolean clear)
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

static void show_recent_apps(void)
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

void init_term_gui(void)
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
static void open_app_later(char *path)
{
    if (path != NULL) {
        run_app = True;
        app_to_open(strdup(path));

        pid_t pid;

        switch (pid = fork()) {
        case -1:
            dump_debug("fork() failed");
            dump_debug_int(errno);

            set_err(ERR_FORK_FAILED);
        case 0: // Child
            /* Change the file mode mask */
            umask(0);

            if (setsid() < 0) {
                dump_debug("setsid() failed");
                dump_debug_int(errno);

                set_err(ERR_SETSID_FAILED);
            }

            if (chdir("/") < 0) {
                dump_debug("chdir() failed");
                dump_debug_int(errno);

                set_err(ERR_CHDIR_FAILED);
            }

            /* Redirect standard files to /dev/null */
            freopen("/dev/null", "r", stdin);
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);

            extern char** environ;
            char *argv[] = {path, NULL};

            execve(argv[0], &argv[0], environ);
        }
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
    } else if (strcmp(name, "^G") == 0) {
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
        case 0:
            // No-op, needed for shortcuts
            break;

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
