#ifndef MBS_H
#define MBS_H

#include <stdint.h>
#include <ncurses.h>

struct stats 
{
    uint64_t rx_bytes;           /* Bytes received */
    uint64_t tx_bytes;           /* Bytes trasmitted */
};

#define FLAG_VERBOSE   (1 << 0)  /* 00000001 */
#define FLAG_COUNTDOWN (1 << 1)  /* 00000010 */
#define FLAG_ASCII     (1 << 2)  /* 00000100 */
#define FLAG_EXIT_ON_0 (1 << 3)  /* 00001000 */

struct mbs
{
    struct stats snapshot;       /* Last TX RX value pair read */
    struct stats used;           /* Data used since the command was launched */
    uint64_t     balance;        /* Current data balance estimate (in bytes) */
    uint8_t      flags;          /* Bit flags */
    char        *ifa_name;       /* Network interface name */
    WINDOW      *win;            /* ncurses window */
};

char *to_human_readable (double bytes, char *buf);
int parse_bytes (const char *str, uint64_t *result);
void mbs_getopt (int argc, char *argv[], struct mbs *s);
int mbs_poll_interfaces (struct mbs *s, struct stats *stats);

#endif 
