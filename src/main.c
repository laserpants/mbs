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
#define _BSD_SOURCE 

#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mbs.h"
#include "window.h"

static volatile bool loop = true;

static void 
sig_handler (int signo) 
{
    if (signo != SIGINT)
        return;

    loop = false;
}

int
main (int argc, char *argv[])
{
    struct timeval tv;
    fd_set s_rd;

    struct mbs state = {
        { 0, 0 },  /* snapshot */
        { 0, 0 },  /* used */
        0,         /* balance */
        0,         /* flags */
        NULL,      /* ifa_name */
        NULL       /* WINDOW */
    };

    struct stats stats = { 0, 0 };

    mbs_getopt (argc, argv, &state);

    if (-1 == mbs_poll_interfaces (&state, &stats))
    {
        fprintf (stderr, "No such interface: %s\n", state.ifa_name);
        free (state.ifa_name);
        return EXIT_FAILURE;
    } 

    state.snapshot = stats;

    if (state.flags & FLAG_VERBOSE)
        printf ("Monitoring network interface %s.\n", state.ifa_name);

    setlocale (LC_ALL, "");
    initscr ();

    if (state.flags & FLAG_COUNTDOWN)
    {
        state.win = newwin (5, 82, 0, 0);
    }
    else
    {
        state.win = newwin (3, 82, 0, 0);
    }

    if (NULL == state.win)
    {
        fprintf (stderr, "Error initialising ncurses.\n");
        endwin ();
        free (state.ifa_name);
        return EXIT_FAILURE;
    }

    curs_set (0);
    timeout (0);  /* This is so that getch doesn't block. */

    signal (SIGINT, sig_handler);

    /* Run main loop until SIGINT signal is received */
    while (true == loop)
    {
        if ('q' == getch ()) /* ...or 'q' is pressed */
            break;

        if (-1 == mbs_poll_interfaces (&state, &stats))
        {
            if (state.flags & FLAG_NO_EXIT)
            {
                state.snapshot.rx_bytes = 0;
                state.snapshot.tx_bytes = 0;

                werase (state.win);

                box (state.win, 0, 0);
                wmove (state.win, 0, 2);
                wattron (state.win, A_BOLD);
                wprintw (state.win, " mbs ");
                wattroff (state.win, A_BOLD);

                wmove (state.win, 2, 2);
                wprintw (state.win, "Interface %s is gone.", state.ifa_name);
                wrefresh (state.win);
            }
            else
            {
                curs_set (1);
                delwin (state.win);
                endwin ();

                fprintf (stderr, "Interface %s is gone.\n", state.ifa_name);

                free (state.ifa_name);

                return -1;
            }
        } 
        else
        {
            const uint64_t tx_diff = stats.tx_bytes - state.snapshot.tx_bytes,
                           rx_diff = stats.rx_bytes - state.snapshot.rx_bytes;

            state.used.tx_bytes += tx_diff;
            state.used.rx_bytes += rx_diff;

            state.snapshot = stats;

            const uint64_t diff = tx_diff + rx_diff;

            if (state.flags & FLAG_COUNTDOWN)
            {
                if (state.balance > diff)
                    state.balance -= diff;
                else
                    state.balance = 0;
            }

            draw_window (&state, !!tx_diff, !!rx_diff);

            if ((state.flags & FLAG_COUNTDOWN) 
             && !state.balance 
             && !(state.flags & FLAG_NO_EXIT)) 
                break;
        }

        FD_ZERO (&s_rd);
        FD_SET (fileno (stdin), &s_rd); 

        tv.tv_sec = 0;
        tv.tv_usec = 200000;

        select (fileno (stdin) + 1, &s_rd, NULL, NULL, &tv);
    }

    curs_set (1);
    delwin (state.win);
    endwin ();

    if ((state.flags & FLAG_COUNTDOWN) && !state.balance)
    {
        printf ("Data limit exceeded.\n");
    }
    else 
    {
        printf ("Terminated!\n");
    }

    free (state.ifa_name);

    return 0;
}
