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

/**
 * @mainpage Welcome!
 *
 * Mbs is a command line tool to keep track of the amount of data sent and 
 * received over a network interface. It is convenient for monitoring data 
 * usage against a pre-paid data bundle or some other fixed usage limit.
 *
 * @section Building
 *
 * To build the executable and tests, run
 *
 * @code
 * ./configure
 * make
 * make install
 * @endcode
 *
 * @section Usage
 *
 * @code
 * mbs [-vk] [--help] [--version] [--ascii] [-a <amount>] [<interface>]
 * @endcode
 *
 * If no `<interface>` is given, the program will try to automatically find an 
 * active network interface (excluding `lo`).
 *
 * @subsection Examples
 *
 * Specify the amount of data available using the `--available` (`-a`) flag to 
 * run the command in *countdown* mode. The following example specifies a data 
 * limit of 300 KB.
 * 
 * @code
 * mbs -a 300K
 * @endcode
 * 
 * By default, the command will exit once this limit is reached, or if the
 * connection is lost. Use the `--keep-running` flag to modify this behavior.
 * 
 * @code
 * mbs -a 10K --keep-running
 * @endcode
 * 
 * You can also omit the `--available` flag, in which case the command will 
 * run in a simplified mode&mdash;only showing the amount of data used since it 
 * started.
 * 
 * @subsection Persistent sessions
 *
 * When the command is run with the `--persistent` (`-p`) flag present, it will 
 * try to continue from where the last session ended. It does so by reading the 
 * last saved state (sent and received bytes count) from a *stats* file. Note 
 * that this will not work if the kernel's TX RX counters were reset since the 
 * last time the command was run (e.g., after a system reboot).
 *
 * The stats file's location can be explicitly set using the 
 * `--statsfile=<path>` flag. If this flag is not provided, then `$HOME/.mbs` 
 * is used as default path.
 *
 * @section Flags
 *
 * | Flag             | Short option   | Description                             |
 * |------------------|----------------|-----------------------------------------|
 * | `--help`         |                | Display help and exit.                  |   
 * | `--version`      |                | Display version info and exit.          |   
 * | `--verbose`      | `-v`           | Render verbose output.                  |   
 * | `--ascii`        |                | Disable non-ascii Unicode characters.   |   
 * | `--keep-running` | `-k`           | Do not exit when data limit is exceeded or connection is lost. |   
 * | `--persistent`   | `-p`           | Continue from where last session ended. |
 * | `--available`    | `-a`           | Amount of data available to use in your subscription plan or budget. |   
 * | `--statsfile`    |                | Override default statsfile path.        |
 *
 * @section source Source Code
 *
 * * GitHub: https://github.com/laserpants/mbs
 */

/**
 * @file main.c
 * @brief Application entry point and main loop.
 *
 * @author Johannes Hildén <hildenjohannes@gmail.com>
 */
#define _BSD_SOURCE 
#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/**
 * @brief This is the application's main entry point. After initialization,
 * it runs the main loop until a `SIGINT` signal is received, or the user 
 * presses the 'Q' key.
 */
int
main (int argc, char *argv[])
{
    struct timeval tv;
    fd_set         s_rd;
    uint64_t       balance;
    bool           balance_set;

    struct mbs state = {
        { 0, 0 },  /* snapshot */
        { 0, 0 },  /* used */
        0,         /* balance */
        0,         /* flags */
        NULL,      /* ifa_name */
        NULL,      /* statsfile */
        NULL,      /* WINDOW */
        NULL       /* FILE */
    };

    struct stats stats = { 0, 0 };

    /*
     * Initialize the mbs struct from command-line arguments.
     */
    mbs_getopt (argc, argv, &state);

    balance_set = !!(state.flags & FLAG_COUNTDOWN);

    if (state.flags & FLAG_VERBOSE)
        printf ("Using stats file: %s\n", state.statsfile);

    if (-1 == access (state.statsfile, F_OK)) 
    {
        FILE *file;
        file = fopen (state.statsfile, "w+");
        fprintf (file, "0:0:0:0:0");
        fflush (file);
        fclose (file);
        state.flags &= ~FLAG_PERSISTENT;
    } 

    state.file = fopen (state.statsfile, "r+");

    if (NULL == state.file)
    {
        fprintf (stderr, "Error opening stats file '%s'.\n", state.statsfile);
    }

    if (state.flags & FLAG_PERSISTENT)
    {
        if (5 == fscanf (
            state.file, "%"PRIu64":%"PRIu64":%"PRIu64":%"PRIu64":%"PRIu64,
            &state.snapshot.tx_bytes,
            &state.snapshot.rx_bytes,
            &state.used.tx_bytes,
            &state.used.rx_bytes,
            &balance)) 
        {
            if (state.flags & FLAG_VERBOSE)
            {
                printf (
                    "Snapshot TX bytes: %"PRIu64"\n", 
                    state.snapshot.tx_bytes
                );
                printf (
                    "Snapshot RX bytes: %"PRIu64"\n", 
                    state.snapshot.rx_bytes
                );
                printf ("Used TX bytes: %"PRIu64"\n", state.used.tx_bytes);
                printf ("Used RX bytes: %"PRIu64"\n", state.used.rx_bytes);
                printf ("Available: %"PRIu64"\n", balance);
            }

            if (!(state.flags & FLAG_COUNTDOWN))
            {
                state.balance = balance;
                state.flags |= FLAG_COUNTDOWN;
            }
        }
        else
        {
            state.snapshot.tx_bytes = 0;
            state.snapshot.rx_bytes = 0;
            state.used.tx_bytes = 0;
            state.used.rx_bytes = 0;
            state.flags &= ~FLAG_PERSISTENT;
        }
    }

    if (-1 == mbs_poll_interfaces (&state, &stats))
    {
        fprintf (stderr, "No such interface: %s\n", state.ifa_name);
        free (state.ifa_name);
        free (state.statsfile);

        if (NULL != state.file)
            fclose (state.file);

        return EXIT_FAILURE;
    } 

    if (state.flags & FLAG_PERSISTENT)
    {
        if (state.snapshot.tx_bytes > stats.tx_bytes || 
            state.snapshot.rx_bytes > stats.rx_bytes)
        {
            fprintf (
                stderr, 
                "Error: The --persistent flag was provided, but it looks like "
                "the kernel's TX/RX counters were reset since last session.\n"
            );

            free (state.ifa_name);
            free (state.statsfile);

            if (NULL != state.file)
                fclose (state.file);

            return EXIT_FAILURE;
        }

        if (true == balance_set)
        {
            const uint64_t tx_diff = stats.tx_bytes - state.snapshot.tx_bytes,
                           rx_diff = stats.rx_bytes - state.snapshot.rx_bytes;

            state.used.tx_bytes += tx_diff;
            state.used.rx_bytes += rx_diff;

            state.snapshot = stats;
        }
    }
    else
    {
        state.snapshot = stats;
    }

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
        free (state.statsfile);

        if (NULL != state.file)
            fclose (state.file);

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
                free (state.statsfile);

                if (NULL != state.file)
                    fclose (state.file);

                return EXIT_FAILURE;
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

            if (NULL != state.file)
            {
                rewind (state.file);

                if (-1 == ftruncate (fileno (state.file), fprintf (
                    state.file, 
                    "%"PRIu64":%"PRIu64":%"PRIu64":%"PRIu64":%"PRIu64, 
                    stats.tx_bytes,
                    stats.rx_bytes,
                    state.used.tx_bytes,
                    state.used.rx_bytes,
                    state.balance
                )))
                {
                    fprintf (stderr, "Error writing to stats file.");
                }

                fflush (state.file);
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
    free (state.statsfile);

    if (NULL != state.file)
        fclose (state.file);

    return 0;
}
