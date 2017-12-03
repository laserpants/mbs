#define _BSD_SOURCE

#include <ifaddrs.h>
#include <linux/if_link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "argtable3.h"

struct context {
    uint32_t  offset;
    uint32_t  available;
    char     *ifa;
};

struct stats {
    uint32_t  rx_bytes;
    uint32_t  tx_bytes;
};

static int 
poll_interfaces (struct context *ctx, struct stats *stats)
{
    struct ifaddrs *ifa0, *ifa;

    if (getifaddrs (&ifa0) == -1)
    {
        perror ("getifaddrs");
        exit (EXIT_FAILURE);
    }

    ifa = ifa0;

    while (ifa != NULL)
    {
        const int family = ifa->ifa_addr->sa_family;

        if (family == AF_PACKET && 0 == strcmp (ifa->ifa_name, ctx->ifa)) 
        {
            const struct rtnl_link_stats *if_stats = ifa->ifa_data;

            stats->rx_bytes = if_stats->rx_bytes;
            stats->tx_bytes = if_stats->tx_bytes;

            freeifaddrs (ifa0);

            return 0;
        }

        ifa = ifa->ifa_next;
    }

    freeifaddrs (ifa0);

    return -1;
}

int 
main (int argc, char *argv[])
{
    struct context ctx;
    struct stats stats = { 0, 0, NULL };

    ctx.offset = 0;
    ctx.available = 8000;

    ctx.ifa = strdup ("wlp2s0");

    if (-1 == poll_interfaces (&ctx, &stats))
    {
        fprintf (stderr, "No such interface: %s", ctx.ifa);
    } 

    ctx.offset = stats.tx_bytes + stats.rx_bytes;

    while (1)
    {
        if (-1 == poll_interfaces (&ctx, &stats))
        {
            fprintf (stderr, "No such interface: %s", ctx.ifa);
        } 
        else
        {
            uint32_t tot = stats.tx_bytes + stats.rx_bytes;
            uint32_t used = tot - ctx.offset;
            uint32_t left = ctx.available - used;

            printf ("Used: %u\n", used);
            printf ("Left: %u\n", left);
        }

        usleep (100000);
    }

    return 0;
}
