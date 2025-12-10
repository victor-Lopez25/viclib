#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define VICLIB_IMPLEMENTATION
#include "../viclib.h"

typedef struct tree_node tree_node;
struct tree_node {
    int someData;
    tree_node *left;
    tree_node *right;
};

// Test for the memory_arena api
int main()
{
    size_t Size = 1024*1024;
    void *mem = malloc(Size);
    mem_zero(mem, Size);
    
    printf("memory start: %p\n", mem);
    memory_arena Arena;
    ArenaInit(&Arena, Size, mem);
    size_t OriginalSize = ArenaGetRemaining(&Arena);
    printf("total arena size: %d\n", (int)OriginalSize);
    
    PushStruct(&Arena, code_location);
    code_location *Loc2 = PushStruct(&Arena, code_location);
    // it's a big struct so alignment shouldn't affect this
    Assert((u8*)Loc2 - (u8*)mem == sizeof(code_location));
    Assert(ArenaGetRemaining(&Arena) == Size - 2*sizeof(code_location));
    
    // second parameter is if the func should clear the mem to zero
    ArenaClear(&Arena, true);
    Assert(Arena.used == 0);
    
    scratch_arena scratch = ArenaBeginScratch(&Arena);
    memory_arena *TempArena = scratch.Arena;
    
    tree_node StartNode = {0};
    tree_node *CurrNode = &StartNode;
    for(int i = 0; i < 1024; i++)
    {
        switch(i%4) {
            case 0: {
                if(!CurrNode->left) {
                    CurrNode->left = PushStruct(TempArena, tree_node, .Alignment = 4);
                }
                CurrNode->left->someData = i;
            } break;
            case 1: {
                if(!CurrNode->right) {
                    CurrNode->right = PushStruct(TempArena, tree_node, 4);
                }
                CurrNode->right->someData = i;
            } break;
            case 2: {
                if(!CurrNode->left) {
                    CurrNode->left = PushStruct(TempArena, tree_node);
                }
                CurrNode = CurrNode->left;
            } break;
            case 3: {
                if(!CurrNode->right) {
                    CurrNode->right = PushStruct(TempArena, tree_node);
                }
                CurrNode = CurrNode->right;
            } break;
        }
    }
    
    printf("remaining size: %d\n", (int)ArenaGetRemaining(&Arena));
    
    // second parameter is if the func should clear the mem to zero
    ArenaEndScratch(scratch, true);
    
    size_t endSize = ArenaGetRemaining(&Arena);
    printf("remaining size after scratch ended: %d\n", (int)endSize);
    Assert(OriginalSize == endSize);
    
    free(mem);
    
    return 0;
}