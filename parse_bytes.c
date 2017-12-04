#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_bytes.h"

int
parse_bytes (const char *str, uint64_t *result)
{
    char *suffix;
    uint64_t b;

    if (NULL == str || 0 == strlen (str))
    {
        *result = 0;
        return 0;
    }

    b = strtoull (str, &suffix, 10);

    if (0 == strlen (suffix) || 0 == strcmp ("B", suffix))
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
        *result = b << 10;
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
        *result = b << 20;
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
        *result = b << 30;
        return 0;
    }

    fprintf (stderr, "Unrecognized suffix: %s\n", suffix);
    return -1;
}
