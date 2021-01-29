#include <ansi_c_functions.h>
#include <stdlib.h>

// Searches for s1 string on memory
// Returns pointer to string
char* memfindsz(const char* s1, char* start, unsigned int size)
{
    unsigned int i = 0;
    while (i < size && strcmp(start, s1) != 0)
    {
        start++;
        i++;
    }

    if (i < size)
        return start;
    else
        return NULL;
}

// Searches for 32-bit value on memory
// Starting address must be word aligned
// Returns pointer to value
unsigned int* memfindu32(const unsigned int val, unsigned int* start, unsigned int size)
{
    unsigned int i = 0;
    while (i < size && *start != val)
    {
        start++;
        i++;
    }

    if (i < size)
        return start;
    else
        return NULL;
}
