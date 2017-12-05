#include <ncurses.h>
#include "stash.h"
#include "window.h"

void 
draw_window (struct stash *s, bool tx_active, bool rx_active)
{
    box (s->win, 0, 0);

    wmove (s->win, 1, 18);

    wprintw (s->win, "%s", s->ifa_name);

    wmove (s->win, 1, 34);

    if (true == tx_active)
    {
        attron (A_BOLD);
        wprintw (
            s->win, "%s", 
            s->flags & FLAG_ASCII ? 
            "*" : "\u2022"
        );
        attroff (A_BOLD);
    }
    wprintw (s->win, " ");

    wrefresh (s->win);
}
