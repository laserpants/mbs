#include <stdio.h>
#include <stdlib.h>
#include "../parse_bytes.h"

static void
test (char *input, uint64_t match)
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
    test ("1024", 1024);
    test ("1024B", 1024);

    return 0;
}
