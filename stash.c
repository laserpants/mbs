#define _BSD_SOURCE 

#include <ifaddrs.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include "stash.h"
#include "parse_bytes.h"
#include "argtable3/argtable3.h"

static char *
human_readable (double bytes, char *buf)
{
    const char *units[] = {"B", "K", "M", "G", "T"};
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

        if (NULL == ifa->ifa_addr)
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

void stash_parse_args (int argc, char *argv[], struct stash *s)
{
    struct arg_lit *verb, 
                   *help, 
                   *version;

    struct arg_str *iface;
    struct arg_end *end;
    struct arg_str *available;

    int nerrors;

    const char command[] = "stash";

    void *argtable[] = 
    {
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
            "a", "available", "<amount>",
            0, 1, "data available to use in your subscription plan"
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
        printf ("Track amount of data transferred over a network interface\
            \against your data plan.\n\n");
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
        arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));
        exit (EXIT_FAILURE);
    }

    s->verbose = !!verb->count;

    if (-1 == parse_bytes (*available->sval, &s->balance)) 
    {
        arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));
        exit (EXIT_FAILURE);
    }

    s->countdown = !!available->count;

    if (true == s->verbose) 
    {
        if (true == s->countdown)
        {
            char buf[10];

            printf (
                "Running in countdown mode. Available amount: %s\n", 
                human_readable (s->balance, buf)
            ); 
        }
        else
        {
            printf ("Not using countdown mode.\n"); 
        }
    }

    arg_freetable (argtable, sizeof(argtable) / sizeof(argtable[0]));
}
