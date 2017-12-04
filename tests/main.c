#include <stdio.h>
#include <stdlib.h>
#include "../parse_bytes.h"

static void
test_parse_bytes (char *input, uint64_t match)
{
    uint64_t result;

    if (-1 == parse_bytes (input, &result))
    {
        fprintf (stderr, "error: %s\n", input);
        exit (EXIT_FAILURE);
    }
    if (result != match)
    {
        fprintf (stderr, "%lu is not \"%s\"\n", match, input);
        exit (EXIT_FAILURE);
    }
}
int 
main (int argc, char *argv[])
{
    uint64_t result;

    test_parse_bytes ("0", 0);
    test_parse_bytes ("", 0);
    test_parse_bytes (NULL, 0);
    test_parse_bytes ("0B", 0);
    test_parse_bytes ("5B", 5);
    test_parse_bytes ("1024", 1024);
    test_parse_bytes ("1024B", 1024);
    test_parse_bytes ("1KB", 1024);
    test_parse_bytes ("1K", 1024);
    test_parse_bytes ("1k", 1000);
    test_parse_bytes ("1M", 1024*1024);
    test_parse_bytes ("1MB", 1024*1024);
    test_parse_bytes ("1MiB", 1024*1024);
    test_parse_bytes ("1m", 1000*1000);
    test_parse_bytes ("2M", 2*1024*1024);
    test_parse_bytes ("2g", 2*1000000000);
    test_parse_bytes ("4g", 4*1000000000L);
    test_parse_bytes ("4G", 4*1024*1024*1024UL);
    test_parse_bytes ("32G", 32*1024*1024*1024UL);
    test_parse_bytes ("132G", 132*1024*1024*1024UL);
    test_parse_bytes ("230GB", 230*1024*1024*1024UL);
    test_parse_bytes ("246960619520", 230*1024*1024*1024UL);

    if (-1 != parse_bytes ("xxx", &result))
    {
        fprintf (stderr, "xxx should return -1\n");
        exit (EXIT_FAILURE);
    }
    if (-1 != parse_bytes ("100pesos", &result))
    {
        fprintf (stderr, "100pesos should return -1\n");
        exit (EXIT_FAILURE);
    }
    
    printf ("All tests OK!\n");

    return 0;
}
