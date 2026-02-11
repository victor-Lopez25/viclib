#include <stdio.h>

#define VICLIB_IMPLEMENTATION
#include "../viclib.h"

void TestReadEntireFile(void)
{
    size_t Size;
    char *Data = ReadEntireFile(&ArenaTemp, "../src/example.c", &Size);
    if(Data) {
        printf("%.*s\n", (int)Size, Data);
    } else {
        printf("Error: %s\n", VL_GetError());
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
        ReadFileChunk(&Chunk, "../src/example.c", &ChunkSize);
        ChunkIdx++)
    {
        printf("Chunk "U64_Fmt": %.*s\n", ChunkIdx, ChunkSize, tmpBuffer);
    }
    if(VL_HadError()) {
        printf("Error: %s\n", VL_GetError());
    }
}

int main()
{
    TestReadEntireFile();
    TestReadFileChunk();

    return 0;
}
