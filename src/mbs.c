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
#define __STDC_FORMAT_MACROS

#include <ifaddrs.h>
#include <inttypes.h>
#include <limits.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "argtable3/argtable3.h"
#include "mbs.h"

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

static void
set_flag (uint8_t *flags, bool set, uint8_t mask)
{
    true == set ? (*flags |= mask) : (*flags &= ~mask);
}

char *
to_human_readable (double bytes, char *buf)
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

int
parse_bytes (const char *str, uint64_t *result)
{
    char *suffix;
    double b;

    if (NULL == str || 0 == strlen (str))
    {
        *result = 0;
        return 0;
    }

    b = strtod (str, &suffix);

    if (0 == strlen (suffix) || 
        0 == strcmp ("B", suffix) || 
        0 == strcmp ("b", suffix))
    {
        *result = b;
        return 0;
    }
    else if (0 == strcmp ("kB", suffix) || 
             0 == strcmp ("k", suffix))
    {
        *result = 1000 * b;
        return 0;
    }
    else if (0 == strcmp ("KB", suffix) || 
             0 == strcmp ("K", suffix)  || 
             0 == strcmp ("KiB", suffix))
    {
        *result = 1024 * b;
        return 0;
    }
    else if (0 == strcmp ("mB", suffix) || 
             0 == strcmp ("m", suffix))
    {
        *result = 1000 * 1000 * b;
        return 0;
    }
    else if (0 == strcmp ("MB", suffix) || 
             0 == strcmp ("M", suffix)  ||
             0 == strcmp ("MiB", suffix))
    {
        *result = 1024 * 1024 * b;
        return 0;
    }
    else if (0 == strcmp ("gB", suffix) || 
             0 == strcmp ("g", suffix))
    {
        *result = 1000 * 1000 * 1000 * b;
        return 0;
    }
    else if (0 == strcmp ("GB", suffix) || 
             0 == strcmp ("G", suffix)  ||
             0 == strcmp ("GiB", suffix))
    {
        *result = 1024 * 1024 * 1024 * b;
        return 0;
    }

    fprintf (stderr, "Unrecognized suffix: %s\n", suffix);
    return -1;
}

void 
mbs_getopt (int argc, char *argv[], struct mbs *s)
{
    struct arg_lit *verb, 
                   *help, 
                   *version,
                   *ascii,
                   *keep_running,
                   *persistent;

    struct arg_str *iface;
    struct arg_end *end;
    struct arg_str *available,
                   *statsfile;

    int nerrors;
    const char command[] = "mbs";

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
        ascii = arg_litn (
            NULL, "ascii", 
            0, 1, "disable Unicode output"
        ),
        keep_running = arg_litn (
            "k", "keep-running", 
            0, 1, 
            "don't exit when data limit is exceeded or connection is lost"
        ),
        persistent = arg_litn (
            "p", "persistent", 
            0, 1, "continue from where last session ended"
        ),
        available = arg_strn (
            "a", "available", "<amount>",
            0, 1, "data available to use in your subscription plan or budget"
        ),
        statsfile = arg_strn (
            NULL, "statsfile", "<path>",
            0, 1, "stats file location (for persistent sessions)"
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
        printf ("Track amount of data transferred over a network interface "
            "against your data plan, or a set budget.\n\n");
        arg_print_glossary (stdout, argtable, "  %-25s %s\n");
        arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));
        exit (EXIT_SUCCESS);
    }

    if (version->count > 0)
    {
        printf ("mbs version 0.1.2\n");
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

    if (-1 == parse_bytes (*available->sval, &s->balance)) 
    {
        arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));
        exit (EXIT_FAILURE);
    }

    if (statsfile->count > 0)
    {
        s->statsfile = strdup (*statsfile->sval);
    }
    else
    {
        char file[PATH_MAX];
        snprintf (file, PATH_MAX, "%s/.mbs", getenv ("HOME"));
        s->statsfile = strdup (file);
    }

    set_flag (&s->flags, !!verb->count, FLAG_VERBOSE);
    set_flag (&s->flags, !!available->count, FLAG_COUNTDOWN);
    set_flag (&s->flags, !!ascii->count, FLAG_ASCII);
    set_flag (&s->flags, !!keep_running->count, FLAG_NO_EXIT);
    set_flag (&s->flags, !!persistent->count, FLAG_PERSISTENT);

    if (s->flags & FLAG_VERBOSE) 
    {
        if (s->flags & FLAG_COUNTDOWN)
        {
            char buf[10];

            printf (
                "Running in countdown mode. Available data: %s\n", 
                to_human_readable (s->balance, buf)
            ); 
        }
        else
        {
            printf ("Not using countdown mode.\n"); 
        }
    }

    arg_freetable (argtable, sizeof(argtable) / sizeof(argtable[0]));
}

int 
mbs_poll_interfaces (struct mbs *s, struct stats *stats)
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
