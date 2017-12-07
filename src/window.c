/*
 * Copyright (c) 2017 Johannes Hildén <hildenjohannes@gmail.com>
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of copyright holder nor the names of other
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <ncurses.h>
#include "window.h"

void 
draw_window (struct mbs *s, bool tx_active, bool rx_active)
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

    wmove (s->win, 0, 2);

    wattron (s->win, A_BOLD);
    wprintw (s->win, " mbs ");
    wattroff (s->win, A_BOLD);

    /* Interface name */

    if (s->flags & FLAG_COUNTDOWN)
    {
        wmove (s->win, 1, 18);
    }
    else
    {
        wmove (s->win, 1, 17);
    }

    wprintw (s->win, "%s", s->ifa_name);

    /* TX */

    wmove (s->win, 1, 31);

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

    wmove (s->win, 1, 47);

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

    /* Exit */

    wmove (s->win, 1, 63);
    wprintw (s->win, "Press 'q' to exit");

    /* Used */

    if (s->flags & FLAG_COUNTDOWN)
    {
        wmove (s->win, 2, 2);
    }
    else
    {
        wmove (s->win, 1, 2);
    }

    wprintw (s->win, "Used: ");

    wattron (s->win, A_BOLD);
    wprintw (s->win, "%s", used_str);
    wattroff (s->win, A_BOLD);

    /* Left */

    if (s->flags & FLAG_COUNTDOWN)
    {
        wmove (s->win, 3, 2);

        wprintw (s->win, "Left: ");

        if (r > 0 || !(s->flags & FLAG_NO_EXIT))
        {
            wattron (s->win, A_BOLD);
            wprintw (s->win, "%s", available_str);
            wattroff (s->win, A_BOLD);
        }
        else
        {
            wprintw (s->win, "-");
        }
    }

    /* Bar */

    if (s->flags & FLAG_COUNTDOWN)
    {
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
            for (i = 0; i < 63 * r - 1; ++i)
                wprintw (s->win, "\u2588");
            for (; i < 63 - 1; ++i)
                wprintw (s->win, "\u2591");
        }
    }

    /* Refresh */

    wrefresh (s->win);
}
