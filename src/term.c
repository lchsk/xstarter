#include <ncurses.h>
#include <menu.h>
#include <form.h>

#include "term.h"
#include "scan.h"
#include "utils.h"

GQueue* tmp = NULL;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	2
#define KEY_ESCAPE 27
#define KEY_ENTER 10
MENU *my_menu;
ITEM **my_items;
int n_choices;
int c;
WINDOW *my_menu_win;
#define MAX_INPUT_LENGTH 20
char query[MAX_INPUT_LENGTH];
int qlen = 0;

char *choices[] = {
(char *)NULL,
};

FIELD *field[2];
FORM  *my_form;

int rem = 0;

static int max_rows;
static int max_cols;
static int _open_app = 0;

void printf_results()
{
    for (int i = 0; i < g_queue_get_length(tmp); i++) {
        char* t = g_queue_peek_nth(tmp, i);
        printf("%s\n", t);
    }
}

void prepare_for_new_results() {
    unpost_menu(my_menu);
    _nc_Disconnect_Items(my_menu);

    for (int i = 0; i < n_choices; i++) {
        free_item(my_items[i]);
    }
    free(my_items);
}

void search(char* query)
{
    g_queue_clear(tmp);

    GQueue* cache = get_cache();

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        char* t = g_queue_peek_nth(cache, i);
        if (strstr(t, query) != NULL) {
            g_queue_push_tail(tmp, (t));
        }
    }

}

void set_items()
{
    rem = 1;
    int tmp_len = 3;
    unpost_menu(my_menu);
    char* path;
    int i = 0;
    int items_found = g_queue_get_length(tmp);

        for(int i = 0; i < n_choices; ++i) {
            free_item(my_items[i]);
        }

        n_choices = items_found + 1;
        my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));

        for (int i = 0; i < g_queue_get_length(tmp); i++) {
            char* t = g_queue_peek_nth(tmp, i);

            my_items[i] = new_item(
                strdup
                (t),
            "");
        }

        set_menu_items(my_menu, my_items);
        post_menu(my_menu);
    }
    refresh();

void create_menu()
{
}
void update_menu()
{
    int start = 0;
   int end = 0;
   int results_cnt = g_queue_get_length(tmp);

   if(results_cnt == 0)
   {
      no_results();
      return;
   }
   my_items = (ITEM **) calloc(results_cnt + 1,sizeof(ITEM *));
   for (int i = 0; i < g_queue_get_length(tmp); i++) {
       char* t = g_queue_peek_nth(tmp, i);

       my_items[i] = new_item(t, (char*) NULL);
   }
   my_items[results_cnt] = new_item((char *) NULL, (char *) NULL);
   set_menu_items(my_menu, my_items);
   post_menu(my_menu);
   refresh();
   n_choices = results_cnt;
}
void no_results()
{
    my_items = (ITEM **) calloc(2,sizeof(ITEM *));
   char * choices[] = {"NO RESULTS", (char *) NULL};
   my_items[0] = new_item(choices[0], (char *) NULL);
   my_items[1] = new_item((char *) NULL, (char *) NULL);
   set_menu_items(my_menu, my_items);
   post_menu(my_menu);
   // wrefresh(list_window);
   refresh();
   n_choices = 2;
}

 void remove_items()
{
    if (rem == 0) return;

    for(int i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
}

void init_term_gui()
{
    set_escdelay(25);
    int i;

    initscr();
    // start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    getmaxyx(stdscr, max_rows, max_cols);

    field[0] = new_field(
        1, // columns?
        10, // width
        1, // pos y
        0, // pos x
        0,
        0
    );
    field[1] = NULL;

    my_form = new_form(field);
    post_form(my_form);
    refresh();

    my_items = (ITEM **) calloc(1,sizeof(ITEM *));
    // char * choices[] = {(char *) NULL};
    // my_items[0] = new_item(choices[0], (char *) NULL);
    my_items[0] = new_item((char *) NULL, (char *) NULL);
    my_menu = new_menu((ITEM**) my_items);
    // set_menu_win(my_menu, list_window);
    // der_window = derwin(list_window, h-2,w-2, 1, 1);
    //set_menu_sub(my_menu, derwin(list_window, h-2,w-2, 1, 1));
    // set_menu_sub(my_menu, der_window);
    // set_menu_format(my_menu, h-2, 1);

    my_menu_win = newwin(
        30, // rows
        max_cols, // cols
        3,
        0
    );
    keypad(my_menu_win, TRUE);

    set_menu_win(my_menu, my_menu_win);
    // set_menu_sub(my_menu, derwin(my_menu_win, 1000, 38, 1, 1));
    set_menu_format(my_menu, 6, 1);

    set_menu_mark(my_menu, " ");

    box(my_menu_win, 0, 0);
    wborder(my_menu_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');

    // print_in_middle(my_menu_win, 1, 0, 40, "My Menu", COLOR_PAIR(1));
    // mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
    // mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
    // mvwaddch(my_menu_win, 2, 39, ACS_RTEE);

    post_menu(my_menu);
    n_choices = 2;
    wrefresh(my_menu_win);

    attron(COLOR_PAIR(2));
    mvprintw(0, 0, "This is xstarter. Start typing to search");
    mvprintw(LINES - 1, 0, "Arrow keys to navigate, <enter> to open, <esc> to quit");
    attroff(COLOR_PAIR(2));
    refresh();
}

void free_term_gui()
{
    unpost_menu(my_menu);
    free_menu(my_menu);

    for (int i = 0; i < n_choices; i++) {
        free_item(my_items[i]);
    }
    free(my_items);

    unpost_form(my_form);
    free_form(my_form);
    free_field(field[0]);

    endwin();
}

void init_search()
{
    tmp = g_queue_new();
}

void free_search()
{
    if (tmp != NULL)
        g_queue_free(tmp);
}

static void
_set_app_to_open(int id)
{
    char* t = g_queue_peek_nth(tmp, id);
    // popen(t, "w");
    _open_app = 1;
    app_to_open(strdup(t));
    // _app_path = strdup(t);
}

void run_term()
{
    move(1, 0);
    while((c = getch()) != KEY_ESCAPE)
    {
        switch(c)
        {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case KEY_ENTER:
                _set_app_to_open(item_index(current_item(my_menu)));
                // pos_menu_cursor(my_menu);
                break;
            case KEY_NPAGE:
                menu_driver(my_menu, REQ_SCR_DPAGE);
                break;
            case KEY_PPAGE:
                menu_driver(my_menu, REQ_SCR_UPAGE);
                break;
            case KEY_BACKSPACE:
                if (qlen >= 1) {
                    form_driver(my_form, REQ_DEL_PREV);
                    qlen--;
                    form_driver(my_form, REQ_VALIDATION);
                    snprintf(query, MAX_INPUT_LENGTH, "%s", field_buffer(field[0], 0));
                    // char kwery2[qlen];
                    char* kwery2 = malloc(qlen);
                    memcpy(kwery2, query, qlen);
                    // handle_input(c, -1);
                    // mvprintw(1, 0, query);
                    // search(kwery2);
                    prepare_for_new_results();
                    search((kwery2));
                    update_menu();
                    // remove_items();
                    // set_items();
                    // move(1, qlen);
                    free(kwery2);
                }
                break;
            default:
                form_driver(my_form, c);
                form_driver(my_form, REQ_VALIDATION);
                snprintf(query, MAX_INPUT_LENGTH, "%s", field_buffer(field[0], 0));
                qlen++;
                char kwery[qlen];
                memcpy(kwery, query, qlen);
                prepare_for_new_results();
                search((kwery));
                update_menu();
        }

        wrefresh(my_menu_win);

        if (_open_app == 1) {
            break;
        }
    }
}
