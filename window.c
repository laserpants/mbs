#include <ncurses.h>
#include "megs.h"
#include "window.h"

void 
draw_window (struct megs *s, bool tx_active, bool rx_active)
{
    int i;
    double tot = s->balance + s->used.tx_bytes + s->used.rx_bytes, 
           r   = tot > 0 ? s->balance / tot : 0;

    char available_str[10], used_str[10], used_tx_str[10], used_rx_str[10];

    to_human_readable (s->balance, available_str);
    to_human_readable (s->used.tx_bytes + s->used.rx_bytes, used_str);

    to_human_readable (s->used.tx_bytes, used_tx_str);
    to_human_readable (s->used.rx_bytes, used_rx_str);

    werase (s->win);

    /* Frame */

    box (s->win, 0, 0);

    wmove (s->win, 0, 4);

    wattron (s->win, A_BOLD);
    wprintw (s->win, " megs ");
    wattroff (s->win, A_BOLD);

    /* Interface name */

    wmove (s->win, 1, 18);

    wprintw (s->win, "%s", s->ifa_name);

    /* TX */

    wmove (s->win, 1, 34);

    if (true == tx_active)
    {
        wattron (s->win, A_BOLD);
        wprintw (
            s->win, "%s", 
            s->flags & FLAG_ASCII ? 
            "*" : "\u2022"
        );
        wattroff (s->win, A_BOLD);
    }
    else
    {
        wprintw (s->win, " ");
    }

    wprintw (s->win, " ");

    if (!(s->flags & FLAG_ASCII))
        wprintw (s->win, "\u2196 ");

    wprintw (s->win, "TX: ");

    wattron (s->win, A_BOLD);
    wprintw (s->win, "%s", used_tx_str);
    wattroff (s->win, A_BOLD);

    /* RX */

    wmove (s->win, 1, 50);

    if (true == rx_active)
    {
        wattron (s->win, A_BOLD);
        wprintw (
            s->win, "%s", 
            s->flags & FLAG_ASCII ? 
            "*" : "\u2022"
        );
        wattroff (s->win, A_BOLD);
    }
    else
    {
        wprintw (s->win, " ");
    }

    wprintw (s->win, " ");

    if (!(s->flags & FLAG_ASCII))
        wprintw (s->win, "\u2199 ");

    wprintw (s->win, "RX: ");

    wattron (s->win, A_BOLD);
    wprintw (s->win, "%s", used_rx_str);
    wattroff (s->win, A_BOLD);

    /* Used */

    wmove (s->win, 2, 2);

    wprintw (s->win, "Used: ");

    wattron (s->win, A_BOLD);
    wprintw (s->win, "%s", used_str);
    wattroff (s->win, A_BOLD);

    /* Left */

    wmove (s->win, 3, 2);

    wprintw (s->win, "Left: ");

    wattron (s->win, A_BOLD);
    wprintw (s->win, "%s", available_str);
    wattroff (s->win, A_BOLD);

    /* Bar */

    wmove (s->win, 3, 18);

    if (s->flags & FLAG_ASCII)
    {
        wprintw (s->win, "[");
        for (i = 0; i < 61 * r - 1; ++i)
            wprintw (s->win, "=");
        wmove (s->win, 3, 79);
        wprintw (s->win, "]");
    }
    else
    {
        wprintw (s->win, "[");
        for (i = 0; i < 61 * r - 1; ++i)
            wprintw (s->win, "=");
        wmove (s->win, 3, 79);
        wprintw (s->win, "]");
    }

    /* Refresh */

    wrefresh (s->win);
}
