#include <ncurses.h>
#include "stash.h"
#include "window.h"

void 
draw_window (struct stash *s, bool tx_active, bool rx_active)
{
    wrefresh (s->win);
}
