#ifndef STASH_H
#define STASH_h

#include <stdbool.h>
#include <stdint.h>

struct stats 
{
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

struct stash
{
    struct stats snapshot;    /* Last TX RX value pair read */
    struct stats used;        /* Data used since the command was launched */
    uint64_t     balance;     /* Current data balance estimate (in bytes) */
    bool         verbose;     /* Verbose mode flag */
    bool         countdown;   /* Countdown mode flag */
    char        *ifa_name;    /* Network interface name */
};

void stash_parse_args (int argc, char *argv[], struct stash *s);

#endif 
