#include <windows.h>
#include <stdio.h>

#define VICLIB_IMPLEMENTATION
#include "viclib.h"

int main()
{
    size_t Size;
    char *Data = ReadEntireFile("../src/test_windows.c", &Size);
    if(Data) {
        printf("%.*s\n", (int)Size, Data);
    }
    else {
        printf("Error: %d\n", ErrorNumber);
    }
    
    return 0;
}