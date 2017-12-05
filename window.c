#include <ncurses.h>
#include "stash.h"
#include "window.h"

void 
draw_window (struct stash *s, bool tx_active, bool rx_active)
{
    wmove (s->win, 1, 1);

    wprintw (s->win, "xx");

    wrefresh (s->win);
}
