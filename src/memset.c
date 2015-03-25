// Memset implementation

#include "memset.h"

void *memset(void *s, int c, size_t n)
{
   // TODO: IMPLEMENT THIS FUNCTION
    unsigned char* ptr = s;
    int i; 
    for(i = 0; i < n; i++){
        unsigned char temp = (unsigned char)c;
        *ptr = temp;
        ptr++;
         
    }
    return s; 
}
