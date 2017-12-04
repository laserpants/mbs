#include <stdio.h>
#include "bar.h"
#include "stash.h"

#define CSI "\x1B["

void
bar_draw_box (void)
{
    int i, j;

    printf (CSI "2K"); /* Clear line */

    printf("\u250C"); 

    for (i = 0; i < 79; ++i)
        printf("\u2500"); 

    printf("\u2510"); 

    printf (CSI "44D"); 
    printf(" stash "); 

    for (j = 0; j < 2; ++j)
    {
        printf (CSI "1E"); /* Move cursor to beginning of next line */
        printf (CSI "2K"); /* Clear line */

        printf("\u2502"); 

        for (i = 0; i < 79; ++i)
            printf("  "); 

        printf("\u2502"); 
    }

    printf (CSI "1E"); /* Move cursor to beginning of next line */
    printf (CSI "2K"); /* Clear line */

    printf("\u2514"); 

    for (i = 0; i < 79; ++i)
        printf("\u2500"); 

    printf("\u2518"); 

    printf (CSI "2F"); 
}

void 
bar_render (struct stash *s)
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
    
    printf (
        "Left: %s", 
        available_str
    );

    printf (CSI "18G"); /* Set column */

    //putchar ('|');

    for (i = 0; i < 62 * r - 1; ++i)
        printf ("\u25AE");
        //printf ("⚫");
        //printf ("\u25CF");
        //printf ("\u25C2");
        //putchar ('=');

    //if (i > 0)
    //{
    //    printf (CSI "1D"); /* Move left */
    //    //printf ("\u00B7");
    //    //putchar ('o');
    //}

    if (r > 0)
    {
        for (; i < 62 - 1; ++i)
            printf ("\u25AF");
            //printf ("⚪");
            //printf ("\u25CB");
            //printf ("\u25C3");
            //putchar (' ');
    }
    else
    {
        printf ("\u2716");
        for (i = 0; i < 60; ++i)
            printf (" ");
    }

    printf ("  \u2502");

    printf (CSI "1E"); /* Move cursor to beginning of next line */
    printf (CSI "2C"); /* Move right */
    printf (CSI "0K"); /* Clear line */

    printf (
        "Used: %s", 
        used_str
    );

    printf (CSI "18G"); /* Set column */
    printf ("TX: %s", used_tx_str); 

    printf (CSI "38G"); /* Set column */
    printf ("RX: %s", used_rx_str); 

    printf (CSI "59G"); /* Set column */
    printf ("Press Ctrl+C to exit  \u2502");

    printf (CSI "1E"); /* Move cursor to beginning of next line */
    printf (CSI "2C"); /* Move right */

    printf (CSI "2F"); /* Move cursor to beginning of {n} lines up */

    fflush (stdout);
}

void 
bar_restore_term (void)
{
    printf (CSI "3E");  /* Move down {n} lines */
    fflush (stdout);
}
