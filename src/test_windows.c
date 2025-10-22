#include <windows.h>
#include <stdio.h>

#define VICLIB_IMPLEMENTATION
#include "../viclib.h"

void TestReadEntireFile(void)
{
    size_t Size;
    char *Data = ReadEntireFile("../src/test_windows.c", &Size);
    if(Data) {
        printf("%.*s\n", (int)Size, Data);
    }
    else {
        printf("Error: %d\n", ErrorNumber);
    }
}

void TestReadFileChunk(void)
{
    u8 tmpBuffer[128];
    file_chunk Chunk = {
        .Buffer = tmpBuffer,
        .BufferSize = sizeof(tmpBuffer),
    };

    u32 ChunkSize;
    for(size_t ChunkIdx = 0;
        ReadFileChunk(&Chunk, "../src/test_windows.c", &ChunkSize);
        ChunkIdx++)
    {
        printf("Chunk %lld: %.*s\n", ChunkIdx, ChunkSize, tmpBuffer);
    }
    if(ErrorNumber != 0) {
        printf("Error: %d\n", ErrorNumber);
    }
}

int main()
{
    //TestReadEntireFile();
    TestReadFileChunk();

    return 0;
}