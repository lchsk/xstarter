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

#define MAX_INPUT_LENGTH (20)
#define KEY_ESCAPE (27)
#define KEY_RETURN (10)

// Seemed to help for ncurses 5.9
#define KEY_BACKSPACE_ALTERNATIVE (8)
#define KEY_DELETE (127)

// Needed for key binding, non-standard
#define KEY_CONTROL_RETURN (256)

#define MAX_ITEMS (100)

static const unsigned MAX_Y = 15;
static const unsigned RECENT_APPS_SHOWN = 10;
static const unsigned XS_COL_SEL = 8;

static const unsigned XS_COLOR_PAIR_1 = 1;

static const unsigned MAX_LIST_ITEM_LENGTH = 24;

typedef struct {
    GList *names;
    char query[MAX_INPUT_LENGTH];
    bool run_app;
    unsigned query_len;
} search_t;

typedef struct {
    unsigned row_start;
    unsigned row_end;
} view_t;

typedef struct {
    char *items[MAX_ITEMS];
    unsigned choices_cnt;
    unsigned offset;
    unsigned selected;
    char *error;
} items_list_t;

static search_t status = {
    .names = NULL,
    .query_len = 0,
    .run_app = false
};

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

static items_list_t items_list;

static void erase_view(const view_t *view);
static int read_emacs_keys(const char *name);
static void move_up(void);
static void move_down(void);
static void reset_query(void);
static void open_by_shortcut(int key);
static void clean_line(int line_y);
static void prepare_for_new_results(void);
static void show_menu(void);
static void show_recent_apps(void);

void cache_loaded(void)
{
    erase_view(&view_search_bar);
    mvprintw(0, 0, "");
    refresh();
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

    const config_t *conf = config();

    colour_t col_sel;
    get_rgb(&col_sel, conf->section_colours->selected);

    if (can_change_color()) {
        init_color(XS_COL_SEL, col_sel.r, col_sel.g, col_sel.b);
        init_pair(XS_COLOR_PAIR_1, COLOR_WHITE, XS_COL_SEL);
    } else{
        init_pair(XS_COLOR_PAIR_1, COLOR_WHITE, COLOR_BLUE);
    }

    show_recent_apps();
    prepare_for_new_results();

    show_menu();

    erase_view(&view_search_bar);

    mvprintw(0, 0, "Loading paths...");

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

        case KEY_CONTROL_RETURN:
            status.run_app = true;

            open_app(
                     g_list_nth_data(results,
                                     items_list.selected + items_list.offset),
                     APP_LAUNCH_MODE_TERM);
            break;

        case KEY_RETURN:
            status.run_app = true;

            open_app(
                     g_list_nth_data(
                                     results,
                                     items_list.selected + items_list.offset),
                     APP_LAUNCH_MODE_GUI);

            break;

        case KEY_DELETE:
        case KEY_BACKSPACE:
        case KEY_BACKSPACE_ALTERNATIVE:
            if (! status.query_len)
                continue;

            status.query_len--;

            if (status.query_len) {
                char *new_query = smalloc(status.query_len + 1);
                memcpy(new_query, status.query, status.query_len);
                new_query[status.query_len] = '\0';

                status.query[status.query_len] = 0;

                erase_view(&view_search_bar);
                mvprintw(0, 0, status.query);

                search(new_query, status.query_len);
                prepare_for_new_results();

                free(new_query);
            } else
                reset_query();

            break;

        default:
            if (status.query_len >= MAX_INPUT_LENGTH)
                continue;

            if (! isprint(c))
                continue;

            if (! is_cache_ready())
                continue;

            if (status.query_len == 0 && c == ' ') {
                reset_query();

                continue;
            } else if (status.query_len == 0) {
                clean_line(0);
            }

            status.query[status.query_len] = c;

            mvaddch(0, status.query_len, c);

            status.query_len++;
            char new_query[status.query_len];

            memcpy(new_query, status.query, status.query_len);
            new_query[status.query_len] = '\0';

            if (search(new_query, status.query_len))
                prepare_for_new_results();
        }

        show_menu();
        move(0, status.query_len);
        refresh();

        if (status.run_app)
            break;
    }

    endwin();
}

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

static void clear_menu(void)
{
    for (int i = 0; i < MAX_ITEMS; i++) {
        items_list.items[i] = NULL;
    }
}

static void update_info_bar(void)
{
    erase_view(&view_info_bar);

    if (items_list.choices_cnt > 0) {
        if (items_list.selected >= items_list.choices_cnt) {
            items_list.selected = 0;
            items_list.offset = 0;
        }

        unsigned item = items_list.selected + items_list.offset;

        GList *l = g_list_nth(results, item);

        if (l) {
            char text[100];

            snprintf(text, 100, "Results: %d", items_list.choices_cnt);

            mvprintw(MAX_Y - 2, 0, text);
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

static void show_menu(void)
{
    for (int i = 0; i < 10; i++) {
        wmove(stdscr, i + 2, 0);

        wclrtoeol(stdscr);
    }

    if (items_list.choices_cnt) {
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

static void move_down(void)
{
    if (items_list.selected + items_list.offset >= items_list.choices_cnt - 1)
        return;

    if (items_list.selected >= 9 && items_list.choices_cnt > 10) {
        items_list.offset++;
    } else if (items_list.selected < items_list.choices_cnt - 1)
        items_list.selected++;

    update_info_bar();
}

static void move_up(void)
{
    if (items_list.selected + items_list.offset == 0)
        return;

    if (items_list.selected == 0 && items_list.offset > 0) {
        items_list.offset--;
    } else if (items_list.selected > 0)
        items_list.selected--;

    update_info_bar();
}

static void prepare_for_new_results(void)
{
    const config_t *conf = config();

    clear_menu();

    items_list.choices_cnt = g_list_length(results);

    int cnt = items_list.choices_cnt > MAX_ITEMS ? MAX_ITEMS : items_list.choices_cnt;

    for (int i = 0; i < cnt; i++) {
        GList *l = g_list_nth(results, i);
        char *path = l->data;

        char *name = g_path_get_basename(path);
        status.names = g_list_prepend(status.names, name);

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

    update_info_bar();
}

static void show_recent_apps(void)
{
    int recent_apps_valid = true;

    for (items_list.choices_cnt = 0;
         items_list.choices_cnt < RECENT_APPS_SHOWN;
         items_list.choices_cnt++) {
        if (recent_apps[items_list.choices_cnt] != NULL
            && strcmp(recent_apps[items_list.choices_cnt], "") != 0) {
            for (int i = 0; i < strlen(recent_apps[items_list.choices_cnt]); i++) {
                if (! isprint(recent_apps[items_list.choices_cnt][i])) {
                    recent_apps_valid = false;
                    break;
                }
            }

            if (! recent_apps_valid)
                break;

            results = g_list_append(results, recent_apps[items_list.choices_cnt]);
        }
    }
}

static void open_by_shortcut(int key)
{
    const config_t *conf = config();

    if (conf->section_main->numeric_shortcuts) {
        if (key >= ASCII_1 && key <= ASCII_9) {
            status.run_app = true;

            open_app(
                     g_list_nth_data(
                                     results,
                                     items_list.offset + (key - ASCII_1)),
                     APP_LAUNCH_MODE_GUI);
        } else if (key == ASCII_0) {
            status.run_app = true;

            open_app(
                     g_list_nth_data(
                                     results, RECENT_APPS_SHOWN - 1),
                     APP_LAUNCH_MODE_GUI);
        }
    }
}

static void reset_query(void)
{
    erase_view(&view_search_bar);
    clear_menu();
    strcpy(status.query, "");
    status.query_len = 0;

    g_list_free(results);
    results = NULL;
    items_list.selected = 0;
    items_list.offset = 0;
    show_recent_apps();
    prepare_for_new_results();
}

static int read_emacs_keys(const char *name)
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
    } else if (strcmp(name, "^O") == 0) {
        return KEY_CONTROL_RETURN;
    }

    return -1;
}
