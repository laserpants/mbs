#ifndef STASH_H
#define STASH_H

#include <stdbool.h>
#include <stdint.h>

struct stats 
{
    uint64_t rx_bytes;  /* Bytes received */
    uint64_t tx_bytes;  /* Bytes trasmitted */
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

char *to_human_readable (double bytes, char *buf);
int parse_bytes (const char *str, uint64_t *result);
void stash_parse_args (int argc, char *argv[], struct stash *s);
int stash_poll_interfaces (struct stash *s, struct stats *stats);

#endif 
