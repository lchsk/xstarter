#include "term.h"
#include "scan.h"
#include <ncurses.h>
#include <menu.h>

GQueue* tmp = NULL;

void printf_results()
{
    for (int i = 0; i < g_queue_get_length(tmp); i++) {
        char* t = g_queue_peek_nth(tmp, i);
        printf("%s\n", t);
    }
}

void search(char* query)
{
    g_queue_clear(tmp);

    GQueue* cache = get_cache();

    for (int i = 0; i < g_queue_get_length(cache); i++) {
        char* t = g_queue_peek_nth(cache, i);
        if (strstr(t, query) != NULL) {
            g_queue_push_tail(tmp, t);
        }
    }
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	2
#define KEY_ESCAPE 27
MENU *my_menu;
ITEM **my_items;
int n_choices;
int c;
WINDOW *my_menu_win;
char query[10];
int qlen = 0;

char *choices[] = {
(char *)NULL,
};

int rem = 0;

void set_items()
{
    rem = 1;
    int tmp_len = 3;
    tmp_len = g_queue_get_length(tmp) + 1;
    char* path;
    int i = 0;

    unpost_menu(my_menu);

    char** op = malloc(tmp_len * sizeof(char*));

    for (int i = 0; i < tmp_len; i++)
        op[i] = malloc(100);

    while ((path = g_queue_pop_head(tmp)) != NULL) {
        strcpy(op[i++], strdup(path));
    }

    n_choices = tmp_len;
    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
    for(int i = 0; i < n_choices; ++i) {
        my_items[i] = new_item(strdup(op[i]), "");
    }

    set_menu_items(my_menu, my_items);

    post_menu(my_menu);
    refresh();

    for (int i = 0; i < n_choices; i++)
        free(op[i]);

    free(op);

}

void remove_items()
{
    if (rem == 0) return;

    for(int i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", "asb");
	wattroff(win, color);
	refresh();
}

void init_term_gui()
{
    set_escdelay(25);
    int i;

    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    n_choices = ARRAY_SIZE(choices);
    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));

    for(i = 0; i < n_choices; ++i)
        my_items[i] = new_item(choices[i], "");

    my_menu = new_menu((ITEM **)my_items);

    my_menu_win = newwin(30, 70, 4, 4);
    keypad(my_menu_win, TRUE);

    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 68, 30, 1));
    set_menu_format(my_menu, 6, 1);

    set_menu_mark(my_menu, "* ");

    box(my_menu_win, 0, 0);
    // print_in_middle(my_menu_win, 1, 0, 40, "My Menu", COLOR_PAIR(1));
    mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
    mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
    mvwaddch(my_menu_win, 2, 39, ACS_RTEE);

    // post_menu(my_menu);
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

    remove_items();
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

static void handle_input(int c, int direction)
{
    if (direction == 1) {
        // new character
        query[qlen] = (char) c;
        qlen++;
    } else if (qlen > 0) {
        // character removed
        qlen--;
        mvaddch(1, qlen, ' ');
        query[qlen] = '\0';
    }
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
            case KEY_NPAGE:
                menu_driver(my_menu, REQ_SCR_DPAGE);
                break;
            case KEY_PPAGE:
                menu_driver(my_menu, REQ_SCR_UPAGE);
                break;
            case KEY_BACKSPACE:
                handle_input(c, -1);
                mvprintw(1, 0, query);
                search(query);
                remove_items();
                set_items();
                move(1, qlen);
                break;
            default:
                handle_input(c, 1);
                mvprintw(1, 0, query);
                search(query);
                remove_items();
                set_items();
                move(1, qlen);
        }

        wrefresh(my_menu_win);
    }

}
