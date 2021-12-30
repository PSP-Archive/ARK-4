#include <stdlib.h>
#include "ansi_c_functions.h"

// Missing Libc Function strcasecmp (needed for stricmp to work)
int strcasecmp(const char * a, const char * b)
{
    // Pointer Equality
    if(a == b) return 0;
    
    // NULL Pointer
    if(a == NULL || b == NULL) return -1;
    
    // Comparison Position
    unsigned int i = 0;
    
    // Compare Character
    while(1)
    {
        // Calculate Difference
        int diff = tolower(a[i]) - tolower(b[i]);
        
        // Difference Detected
        if(diff != 0) return diff;
        
        // End of String Detected
        if(a[i] == 0) break;
        
        // Move Position
        i++;
    }
    
    // Equal Strings
    return 0;
}
