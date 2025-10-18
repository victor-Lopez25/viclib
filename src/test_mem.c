#include <stdio.h>
#include <stdlib.h>

#define VICLIB_IMPLEMENTATION
#include "../viclib.h"

typedef struct tree_node tree_node;
struct tree_node {
    int SomeData;
    tree_node *Left;
    tree_node *Right;
};

// Test for the memory_arena api
int main()
{
    size_t Size = 1024*1024;
    void *Mem = malloc(Size);
    mem_zero(Mem, Size);
    
    printf("memory start: %p\n", Mem);
    memory_arena Arena;
    ArenaInit(&Arena, Size, Mem);
    size_t OriginalSize = ArenaGetRemaining(&Arena);
    printf("total arena size: %d\n", (int)OriginalSize);
    
    PushStruct(&Arena, code_location);
    code_location *Loc2 = PushStruct(&Arena, code_location);
    // it's a big struct so alignment shouldn't affect this
    Assert((u8*)Loc2 - (u8*)Mem == sizeof(code_location));
    Assert(ArenaGetRemaining(&Arena) == Size - 2*sizeof(code_location));
    
    // second parameter is if the func should clear the mem to zero
    ArenaClear(&Arena, true);
    Assert(Arena.Used == 0);
    
    scratch_arena Scratch = ArenaBeginScratch(&Arena);
    memory_arena *TempArena = Scratch.Arena;
    
    tree_node StartNode = {0};
    tree_node *CurrNode = &StartNode;
    for(int i = 0; i < 1024; i++)
    {
        switch(i%4) {
            case 0: {
                if(!CurrNode->Left) {
                    CurrNode->Left = PushStruct(TempArena, tree_node, .Alignment = 4);
                }
                CurrNode->Left->SomeData = i;
            } break;
            case 1: {
                if(!CurrNode->Right) {
                    CurrNode->Right = PushStruct(TempArena, tree_node, 4);
                }
                CurrNode->Right->SomeData = i;
            } break;
            case 2: {
                if(!CurrNode->Left) {
                    CurrNode->Left = PushStruct(TempArena, tree_node);
                }
                CurrNode = CurrNode->Left;
            } break;
            case 3: {
                if(!CurrNode->Right) {
                    CurrNode->Right = PushStruct(TempArena, tree_node);
                }
                CurrNode = CurrNode->Right;
            } break;
        }
    }
    
    printf("remaining size: %d\n", (int)ArenaGetRemaining(&Arena));
    
    // second parameter is if the func should clear the mem to zero
    ArenaEndScratch(Scratch, true);
    
    size_t EndSize = ArenaGetRemaining(&Arena);
    printf("remaining size after scratch ended: %d\n", (int)EndSize);
    Assert(OriginalSize == EndSize);
    
    free(Mem);
    
    return 0;
}