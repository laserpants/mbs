#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
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
        { 0, 0 },
        { 0, 0 },
        0,
        false,
        false,
        NULL
    };

    stash_parse_args (argc, argv, &state);

    set_input_mode ();

    set_cursor (false);

    signal (SIGINT, sig_handler);

    /* Run main loop until SIGINT signal is received. */
    while (true == loop)
    {
    }

    set_cursor (true);

    return 0;
}

/*
#define _BSD_SOURCE 

#include <ifaddrs.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "argtable3.h"

#define WIN_WIDTH 50
#define CSI "\x1B["

struct state 
{
    uint32_t  snapshot;
    uint64_t  available;
    uint64_t  used;
    bool      verbose;
    bool      countdown;
    char     *ifa_name;
};

struct stats 
{
    uint32_t  rx_bytes;
    uint32_t  tx_bytes;
};

static volatile int running = true;

static void 
int_handler (int sig) 
{
    running = false;
}

static char *
human_readable (double bytes, char *buf)
{
    const char *units[] = {"B", "K", "M", "G"};
    int i = 0;

    while (bytes >= 1024)
    {
        bytes /= 1024;
        ++i;
    }

    sprintf (buf, "%.*f%s", i, bytes, units[i]);
    return buf;
}

static int
get_default_interface (char **ifa_name)
{
    struct ifaddrs *ifa0, *ifa;

    if (getifaddrs (&ifa0) == -1)
    {
        perror ("getifaddrs");
        return -1;
    }

    for (ifa = ifa0; ifa != NULL; ifa = ifa->ifa_next)
    {
        const unsigned short sa_family = ifa->ifa_addr->sa_family;
        const int ifa_running = ifa->ifa_flags & IFF_RUNNING;

        if (ifa->ifa_addr == NULL)
            continue;

        if (sa_family != AF_INET && sa_family != AF_INET6) 
            continue;

        if ((strcmp ("lo", ifa->ifa_name) == 0) || !ifa_running)
            continue;

        *ifa_name = strdup (ifa->ifa_name);

        freeifaddrs (ifa0);
        return 0;
    }

    freeifaddrs (ifa0);
    return -1;
}

static int 
poll_interfaces (struct state *s, struct stats *stats)
{
    struct ifaddrs *ifa0, *ifa;

    if (getifaddrs (&ifa0) == -1)
    {
        perror ("getifaddrs");
        exit (EXIT_FAILURE);
    }

    for (ifa = ifa0; ifa != NULL; ifa = ifa->ifa_next)
    {
        const unsigned short int sa_family = ifa->ifa_addr->sa_family;

        if (sa_family == AF_PACKET && 0 == strcmp (ifa->ifa_name, s->ifa_name))  
        {
            const struct rtnl_link_stats *if_stats = ifa->ifa_data;

            stats->rx_bytes = if_stats->rx_bytes;
            stats->tx_bytes = if_stats->tx_bytes;

            freeifaddrs (ifa0);
            return 0;
        }
    }

    freeifaddrs (ifa0);
    return -1;
}

static uint64_t 
parse_bytes (const char *str)
{
    char *suffix;
    uint64_t b;

    if (NULL == str || 0 == strlen (str))
        return 0;

    suffix = (char *) str;

    while (*suffix >= '0' && *suffix <= '9')
        ++suffix;

    b = strtoull (str, &suffix, 10);

    if (0 == strlen (suffix))
    {
        return b;
    }
    else if (0 == strcmp ("kB", suffix) || 
             0 == strcmp ("k", suffix))
    {
        return 1000 * b;
    }
    else if (0 == strcmp ("KB", suffix) || 
             0 == strcmp ("K", suffix)  || 
             0 == strcmp ("KiB", suffix))
    {
        return b << 10;
    }
    else if (0 == strcmp ("mB", suffix) || 
             0 == strcmp ("m", suffix))
    {
        return 1000 * 1000 * b;
    }
    else if (0 == strcmp ("MB", suffix) || 
             0 == strcmp ("M", suffix)  ||
             0 == strcmp ("MiB", suffix))
    {
        return b << 20;
    }
    else if (0 == strcmp ("gB", suffix) || 
             0 == strcmp ("g", suffix))
    {
        return 1000 * 1000 * 1000 * b;
    }
    else if (0 == strcmp ("GB", suffix) || 
             0 == strcmp ("G", suffix)  ||
             0 == strcmp ("GiB", suffix))
    {
        return b << 30;
    }

    fprintf (stderr, "Unrecognized suffix: %s", suffix);
    exit (EXIT_FAILURE);
}

static void 
parse_args (int argc, char *argv[], struct state *s)
{
    struct arg_lit *verb, 
                   *help, 
                   *version;
    struct arg_str *iface;
    struct arg_end *end;
    struct arg_str *available;

    int nerrors;
    const char command[] = "stash";

    void *argtable[] = {
        help = arg_litn (
            NULL, "help", 
            0, 1, "display this help and exit"
        ),
        version = arg_litn (
            NULL, "version", 
            0, 1, "display version info and exit"
        ),
        verb = arg_litn (
            "v", "verbose", 
            0, 1, "verbose output"
        ),
        available = arg_strn (
            "b", "available", "<bytes>",
            0, 1, "bytes available to use in your subscription plan"
        ),
        iface = arg_strn (
            NULL, NULL, "<interface>", 
            0, 1, "network interface"
        ),
        end = arg_end (20),
    };

    nerrors = arg_parse (argc, argv, argtable);

    if (help->count > 0)
    {
        printf ("Usage: %s", command);
        arg_print_syntax (stdout, argtable, "\n");
        printf ("Track amount of data transferred over a network interface.\n\n");
        arg_print_glossary (stdout, argtable, "  %-25s %s\n");
        arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));
        exit (EXIT_SUCCESS);
    }

    if (nerrors > 0)
    {
        arg_print_errors (stdout, end, command);
        printf ("Try '%s --help' for more information.\n", command);
        arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));
        exit (EXIT_FAILURE);
    }

    if (strlen (*iface->sval) > 0)
    {
        s->ifa_name = strdup (*iface->sval);
    }
    else if (-1 == get_default_interface (&s->ifa_name))
    {
        fprintf (stderr, "No active network interface found.\n");
        exit (EXIT_FAILURE);
    }

    s->verbose = !!verb->count;
    s->available = parse_bytes (*available->sval); 
    s->countdown = !!available->count;

    if (true == s->verbose) 
    {
        if (true == s->countdown)
        {
            char buf[10];
            printf (
                "Running in countdown mode. Available bytes: %s\n", 
                human_readable (s->available, buf)
            ); 
        }
        else
        {
            printf ("Not using countdown mode.\n"); 
        }
    }

    arg_freetable (argtable, sizeof(argtable) / sizeof(argtable[0]));
}

static void
draw_bar (struct state *state)
{
    int i, j;
    double tot = state->available + state->used, 
           r   = tot > 0 ? state->available / tot : 0;
    char available_buf[10];
    char used_buf[10];

    printf (CSI "1000D");

    j = WIN_WIDTH * r;

    putchar ('|');

    for (i = 0; i < j - 1; ++i)
        putchar ('=');

    for (; i < WIN_WIDTH - 1; ++i)
        putchar (' ');

    putchar ('|');

    printf (" %.*f%%", 1, r*100);

    printf (CSI "0K");
    printf (CSI "1E");

    human_readable (state->available, available_buf);
    human_readable (state->used, used_buf);

    printf (
        "Remaining: %s | Used: %s | Press 'Ctrl+C' to exit", 
        available_buf, used_buf
    );

    printf (CSI "1F");

    fflush (stdout);
}

int 
main (int argc, char *argv[])
{
    struct state state = {
        0,    // snapshot 
        0,    // available 
        0,    // used 
        0,    // verbose 
        0,    // countdown 
        NULL  // ifa_name 
    };

    struct stats stats = { 0, 0, NULL };

    signal (SIGINT, int_handler);

    parse_args (argc, argv, &state);

    printf (CSI "?25l");

    if (-1 == poll_interfaces (&state, &stats))
    {
        fprintf (stderr, "No such interface: %s\n", state.ifa_name);
        exit (EXIT_FAILURE);
    } 

    state.snapshot = stats.tx_bytes + stats.rx_bytes;

    if (true == state.verbose)
        printf ("Using network interface %s.\n", state.ifa_name);

    while (running)
    {
        if (-1 == poll_interfaces (&state, &stats))
        {
            fprintf (stderr, "No such interface: %s", state.ifa_name);
            exit (EXIT_FAILURE);
        } 
        else
        {
            const uint32_t txrx = stats.tx_bytes + stats.rx_bytes;
            const uint32_t diff = txrx - state.snapshot;

            state.used += diff;

            if (state.countdown)
            {
                if (state.available > diff)
                    state.available -= diff;
                else
                    state.available = 0;
            }

            state.snapshot = txrx;

            draw_bar (&state);

            if (state.countdown && !state.available)
                break;
        }

        usleep (100000);
    }

    free (state.ifa_name);

    printf (CSI "?25h");
    printf (CSI "2E");

    if (state.countdown && !state.available)
        printf ("Data limit exceeded.\n");

    return 0;
}
*/
