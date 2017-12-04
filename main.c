#define _BSD_SOURCE 

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "bar.h"
#include "stash.h"

#define CSI "\x1B["

static volatile bool loop = true;

static void 
sig_handler (int signo) 
{
    if (signo != SIGINT)
        return;

    loop = false;
}

struct termios term_attrs;

static void
reset_input_mode (void)
{
    tcsetattr (STDIN_FILENO, TCSANOW, &term_attrs);
}

static void
set_input_mode (void)
{
    struct termios term;

    if (!isatty (STDIN_FILENO))
    {
        fprintf (stderr, "stdin is not a terminal.\n");
        exit (EXIT_FAILURE);
    }

    /* Store terminal attributes so we can restore them later. */
    tcgetattr (STDIN_FILENO, &term_attrs);
    atexit (reset_input_mode);

    tcgetattr (STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);    /* Clear ICANON and ECHO. */
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &term);
}

static void
set_cursor (bool visible)
{
    printf (true == visible ? CSI "?25h" : CSI "?25l");
    fflush (stdout);
}

int
main (int argc, char *argv[])
{
    struct stash state = {
        { 0, 0 },  /* snapshot */
        { 0, 0 },  /* used */
        0,         /* balance */
        false,     /* verbose */
        false,     /* countdown */
        false,     /* ascii */
        NULL       /* ifa_name */
    };

    struct stats stats = { 0, 0 };

    int status = 0;

    stash_parse_args (argc, argv, &state);

    if (-1 == stash_poll_interfaces (&state, &stats))
    {
        fprintf (stderr, "No such interface: %s\n", state.ifa_name);
        free (state.ifa_name);
        return EXIT_FAILURE;
    } 

    state.snapshot = stats;

    if (true == state.verbose)
        printf ("Monitoring network interface %s.\n", state.ifa_name);

    set_input_mode ();

    printf ("\n\n\n\n\n");

    printf (CSI "s");  /* Save cursor */
    printf (CSI "5F"); /* Move up 5 lines */ 

    set_cursor (false);

    signal (SIGINT, sig_handler);

    bar_draw_box ();

    /* Run main loop until SIGINT signal is received. */
    while (true == loop)
    {
        if (-1 == stash_poll_interfaces (&state, &stats))
        {
            fprintf (stderr, "Interface %s is gone.\n", state.ifa_name);
            status = -1;
            goto exit;
        } 
        else
        {
            const uint64_t tx_diff = stats.tx_bytes - state.snapshot.tx_bytes,
                           rx_diff = stats.rx_bytes - state.snapshot.rx_bytes;

            state.used.tx_bytes += tx_diff;
            state.used.rx_bytes += rx_diff;

            state.snapshot = stats;

            const uint64_t diff = tx_diff + rx_diff;

            if (true == state.countdown)
            {
                if (state.balance > diff)
                    state.balance -= diff;
                else
                    state.balance = 0;

                bar_render (&state);

                if (!state.balance)
                    break;
            }
        }

        usleep (40000);
    }

exit:
    bar_restore_term ();

    printf (CSI "u"); /* Unsave cursor */

    if (0 == status && true == state.countdown && !state.balance)
        printf ("Data limit exceeded.\n");
    else if (0 == status)
        printf ("Terminated!\n");

    set_cursor (true);

    free (state.ifa_name);

    return status;
}
