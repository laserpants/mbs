#include <stdio.h>
#include "bar.h"
#include "stash.h"

#define CSI "\x1B["

void
bar_draw_box (void)
{
    int i;

    printf("\u250C");
    for (i = 0; i < 79; ++i)
        printf("\u2500");
    printf("\u2510");

    printf (CSI "79D");      /* Move left */
    printf(" \033[1mstash\033[22m ");

    for (i = 0; i < 3; ++i)
    {
        printf (CSI "1E");   /* Move cursor to beginning of next line */
        printf (CSI "2K");   /* Clear line */
        printf("\u2502");
        printf (CSI "81G");  /* Set column */
        printf("\u2502");
    }

    printf (CSI "1E");       /* Move cursor to beginning of next line */

    printf("\u2514");
    for (i = 0; i < 79; ++i)
        printf("\u2500");
    printf("\u2518"); 

    printf (CSI "3F"); 
}

void 
bar_render (struct stash *s, bool tx, bool rx)
{
    int i;
    double tot = s->balance + s->used.tx_bytes + s->used.rx_bytes, 
           r   = tot > 0 ? s->balance / tot : 0;

    char available_str[10], used_str[10], used_tx_str[10], used_rx_str[10];

    to_human_readable (s->balance, available_str);
    to_human_readable (s->used.tx_bytes + s->used.rx_bytes, used_str);

    to_human_readable (s->used.tx_bytes, used_tx_str);
    to_human_readable (s->used.rx_bytes, used_rx_str);

    printf (CSI "2C"); /* Move right */
    printf (CSI "0K"); /* Clear line */
    
    printf (CSI "18G"); /* Set column */

    printf ("\033[1m%s\033[22m", s->ifa_name);

    printf (CSI "34G"); /* Set column */

    if (true == tx)
        printf (s->flags & FLAG_ASCII ? "* " : "\033[1m\u2022\033[22m ");
    else
        printf ("  ");

    if (!(s->flags & FLAG_ASCII))
        printf ("\u2196 ");

    printf ("TX: \033[1m%s\033[22m", used_tx_str);

    printf (CSI "50G"); /* Set column */

    if (true == rx)
        printf (s->flags & FLAG_ASCII ? "* " : "\033[1m\u2022\033[22m ");
    else
        printf ("  ");

    if (!(s->flags & FLAG_ASCII))
        printf ("\u2199 ");

    printf ("RX: \033[1m%s\033[22m", used_rx_str);

    printf (CSI "66G"); /* Set column */
    printf ("Ctrl+C to exit \u2502");

    printf (CSI "1E"); /* Move cursor to beginning of next line */
    printf (CSI "2C"); /* Move right */
    printf (CSI "0K"); /* Clear line */

    printf ("Used: \033[1m%s\033[22m", used_str);
    printf (CSI "81G"); /* Set column */
    printf ("\u2502");

    printf (CSI "1E"); /* Move cursor to beginning of next line */
    printf (CSI "2C"); /* Move right */
    printf (CSI "0K"); /* Clear line */

    printf ("Left: \033[1m%s\033[22m", available_str);

    printf (CSI "18G"); /* Set column */

    if (s->flags & FLAG_ASCII)
    {
        printf ("[");
        for (i = 0; i < 61 * r - 1; ++i)
            printf ("=");
        printf (CSI "79G"); /* Set column */
        printf ("] \u2502");
    }
    else
    {
        for (i = 0; i < 63 * r - 1; ++i)
            printf ("\u2588");
        for (; i < 63 - 1; ++i)
            printf ("\u2591");
        printf (" \u2502");
    }

    printf (CSI "2F"); /* Move cursor to beginning of {n} lines up */

    fflush (stdout);
}

void 
bar_restore_term (void)
{
    printf (CSI "3E");  /* Move down {n} lines */
    fflush (stdout);
}
