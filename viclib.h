/* date = December 29th 2024 10:12 pm
--Author: Víctor López Cortés
--Usage:
Defines: To have any of these take effect, you must define them _before_ including this file
 - VICLIB_IMPLEMENTATION: If you want to have the implementation (only in one file)
 - READ_ENTIRE_FILE_MAX: If you want to have a max file read size, default is 0xFFFFFFFF (4GB)
 - QUIET_ASSERT: If you want the assertions to add a breakpoint but not print
 - RELEASE_MODE: Have some stuff work faster, right now, assertions get compiled out when this is defined
 - VICLIB_PROC: Define to 'static' or some kind of export as needed
 - VICLIB_TEMP_SIZE: ArenaTemp size, default is 4*1024*1024 bytes
 - VICLIB_NO*: If you want to remove parts of the library:
   - VICLIB_NO_TEMP_ARENA: remove ArenaTemp
   - VICLIB_NO_SORT: remove Sort and all functions used by it
Check ErrorNumber when errors occur.

--Many thanks to the inspirations for this library:
 - Mr4th's 4ed_base_types.h - https://mr-4th.itch.io/4coder (find the file in 'custom' directory)
 - stb header-only libraries - https://github.com/nothings/stb
 - tsoding's string view implementation - https://github.com/tsoding/sv

I modified tsoding's string view so that it doesn't use the stdlib by implementing the couple
functions it uses

LICENCES:
tsoding string view implementation:
Copyright 2021 Alexey Kutepov <reximkut@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

License to viclib.h:
Copyright (c) 2024 Víctor López

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef VICLIB_H
#define VICLIB_H

#if defined(_WIN32)
# define OS_WINDOWS 1
#endif

#if defined(_MSC_VER)
# define COMPILER_CL 1
# define PRAGMA(x) __pragma(x)
# define thread_local __declspec(thread)
#elif defined(__clang__)
# define COMPILER_CLANG 1
# define PRAGMA(x) _Pragma(#x)
# define thread_local __thread
#elif defined(__GNUC__) || defined(__GNUG__)
# define COMPILER_GCC 1
# define PRAGMA(x) _Pragma(#x)
# define thread_local __thread
#else
/* unsupported compiler, to support, define:
 necessary:
 - thread_local
 useful:
 - PUSH_IGNORE_UNINITIALIZED
 - RESTORE_WARNINGS
*/
# define thread_local
#endif

#if defined(__gnu_linux__)
# define OS_LINUX 1
#endif

#if defined(__APPLE__) && defined(__MACH__)
# define OS_MAC 1
#endif

#if !defined(__COLUMN__)
# define __COLUMN__ 0
#endif

#if COMPILER_CL || COMPILER_GCC || COMPILER_CLANG
/* gcc and clang define __FUNCTION__ as a string constant,
 which means it cannot be concatenated to a string literal at compile time */
# define __PROC__ __FUNCTION__
#endif

/* DebugBreakpoint for different platforms.
Implementation by SDL3 (sdl wiki says SDL2 also had this but code says since SDL3.1.3) */
#if defined(SDL_h_)
# define DebugBreakpoint SDL_TriggerBreakpoint()
#elif defined(_MSC_VER)
/* Don't include intrin.h here because it contains C++ code */
extern void __cdecl __debugbreak(void);
# define DebugBreakpoint __debugbreak()
#elif defined(ANDROID)
# include <assert.h>
# define DebugBreakpoint assert(0)
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
# define DebugBreakpoint __asm__ __volatile__ ( "int $3\n\t" )
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__riscv)
# define DebugBreakpoint __asm__ __volatile__ ( "ebreak\n\t" )
#elif (defined(OS_MAC) && (defined(__arm64__) || defined(__aarch64__)) )  /* this might work on other ARM targets, but this is a known quantity... */
# define DebugBreakpoint __asm__ __volatile__ ( "brk #22\n\t" )
#elif defined(OS_MAC) && defined(__arm__)
# define DebugBreakpoint __asm__ __volatile__ ( "bkpt #22\n\t" )
#elif defined(_WIN32) && ((defined(__GNUC__) || defined(__clang__)) && (defined(__arm64__) || defined(__aarch64__)) )
# define DebugBreakpoint __asm__ __volatile__ ( "brk #0xF000\n\t" )
#elif defined(__386__) && defined(__WATCOMC__)
# define DebugBreakpoint { _asm { int 0x03 } }
#elif defined(HAVE_SIGNAL_H) && !defined(__WATCOMC__)
# include <signal.h>
# define DebugBreakpoint raise(SIGTRAP)
#else
/* How do we trigger breakpoints on this platform? */
# define DebugBreakpoint Assert(false)
#endif

#if COMPILER_GCC || COMPILER_CLANG
// gcc and cl (haven't tried clang)
// don't understand how functions like ReadEntireFile work
// but I want to keep the warnings for other functions
# define PUSH_IGNORE_UNINITIALIZED PRAGMA(GCC diagnostic push) PRAGMA(GCC diagnostic ignored "-Wuninitialized")
# define RESTORE_WARNINGS PRAGMA(GCC diagnostic pop)
#elif COMPILER_CL
# define PUSH_IGNORE_UNINITIALIZED PRAGMA(warning(push)) PRAGMA(warning(disable: 4701)) PRAGMA(warning(disable: 4703))
# define RESTORE_WARNINGS PRAGMA(warning(pop))
#else
// compiler specific implementation
# define PUSH_IGNORE_UNINITIALIZED
# define RESTORE_WARNINGS
#endif

// only works with static arrays!
#define ArrayLen(arr) sizeof(arr)/sizeof(arr[0])

#if !defined(stringify)
# define stringify_(a) #a
# define stringify(a) stringify_(a)
#endif
#if !defined(glue)
# define glue_(a,b) a##b
# define glue(a,b) glue_(a,b)
#endif

#ifndef min
# define min(A, B) ((A) < (B) ? (A) : (B))
#endif
#ifndef max
# define max(A, B) ((A) > (B) ? (A) : (B))
#endif

#define fallthrough

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint32_t b32;
typedef float    f32;
typedef double   f64;

#ifndef bool
typedef uint8_t  bool;
#define true 1
#define false 0
#endif

// TODO: Print for different platforms

#if !defined(AssertAlways) || !defined(AssertMsgAlways)
# if defined(SDL_h_)
#  define AssertAlways(e) do{ if(!(e)){ \
        SDL_Log(__FILE__"("stringify(__LINE__)"): Assert fail: "#e "\n"); \
        DebugBreakpoint; } }while(0)
#  define AssertMsgAlways(e, msglit) do{ if(!(e)){ \
        SDL_Log(__FILE__"("stringify(__LINE__)"): " msglit "\n"); \
        DebugBreakpoint; } }while(0)
# elif defined(_INC_STDIO)
#  define AssertAlways(e) do{ if(!(e)){ \
        printf(__FILE__"("stringify(__LINE__)"): Assert fail: "#e "\n"); \
        fflush(stdout); DebugBreakpoint; } }while(0)
#  define AssertMsgAlways(e, msglit) do{ if(!(e)){ \
        printf(__FILE__"("stringify(__LINE__)"): " msglit "\n"); \
        fflush(stdout); DebugBreakpoint; } }while(0)
# else
#  if !defined(QUIET_ASSERT)
#   define QUIET_ASSERT
#   pragma message("Warning: Using quiet assert since stdio.h is not included")
#   define AssertAlways(e) do{ if(!(e)) { DebugBreakpoint; } }while(0)
#   define AssertMsgAlways(e, msg) AssertAlways(e)
#  endif
# endif
#endif // !defined(AssertAlways) || !defined(AssertMsgAlways)

#if RELEASE_MODE
# define Assert(expr)
# define AssertMsg(expr, msg)
#else
# define Assert(expr) AssertAlways(expr)
# define AssertMsg(expr, msg) AssertMsgAlways(expr, msg)
#endif

thread_local u32 ErrorNumber = 0;

#ifndef VLIBPROC
# define VLIBPROC
#endif

////////////////////////////////
// intrinsics
////////////////////////////////

#if defined(_INC_STRING)
# define mem_copy_non_overlapping(dst, src, len) memcpy(dst, src, len)
# define mem_copy(dst, src, len) memmove(dst, src, len)
# define mem_zero(data, len) memset(data, 0, len)
# define mem_compare(str1, str2, count) memcmp(str1, str2, count)
#elif defined(SDL_h_)
# define mem_copy_non_overlapping(dst, src, len) SDL_memcpy(dst, src, len)
# define mem_copy(dst, src, len) SDL_memmove(dst, src, len)
# define mem_zero(data, len) SDL_memset(data, 0, len)
# define mem_compare(str1, str2, count) SDL_memcmp(str1, str2, count)
#else
VLIBPROC void mem_copy_non_overlapping(void *dst, const void *src, size_t len);
VLIBPROC void mem_copy(void *dst, const void *src, size_t len);
VLIBPROC void mem_zero(void *data, size_t len);
VLIBPROC int mem_compare(const void *str1, const void *str2, size_t count);
#endif
#define ZeroStruct(S) mem_zero(&(S), sizeof(S))

////////////////////////////////

typedef struct {
    const char *Data;
    size_t Len;
} view;
#define VIEW(cstr_lit) view_FromParts((cstr_lit), sizeof(cstr_lit) - 1)
#if defined(__cplusplus)
#define VIEW_STATIC(cstr_lit) {(const char*)(cstr_lit), sizeof(cstr_lit) - 1}
#else
#define VIEW_STATIC(cstr_lit) (view){(const char*)(cstr_lit), sizeof(cstr_lit) - 1}
#endif
#define VIEW_FMT "%.*s"
#define VIEW_ARG(v) (int)(v).Len, (v).Data

#ifndef VIEWPROC
# define VIEWPROC VLIBPROC
#endif

#ifndef strlen
size_t strlen(const char *s);
#endif // strlen

int is_space(int _c); // only checks ascii space characters
VIEWPROC view view_FromParts(const char *Data, size_t Count);
VIEWPROC view view_FromCstr(const char *Cstr);
VIEWPROC view view_Slice(view A, size_t start, size_t end); // won't include end -> [start, end)
VIEWPROC int  view_Compare(view A, view B); // result = A - B
VIEWPROC bool view_Eq(view A, view B);
VIEWPROC bool view_StartsWith(view v, view Start);
VIEWPROC const char *view_Contains(view Haystack, view Needle); // result = pointer to where the needle is in haystack or null
VIEWPROC bool view_ContainsCharacter(view v, char c);
#define view_EndWith view_EndsWith /* in case of singular/plural annoyance */
VIEWPROC bool view_EndsWith(view v, view End);
VIEWPROC view view_ChopByDelim(view *v, char Delim);
VIEWPROC view view_ChopByAnyDelim(view *v, view Delims, char *Delimiter); // checks for any character in Delims, stores found delimiter in Delimiter
VIEWPROC view view_ChopByView(view *v, view Delim); // full view is the delim
VIEWPROC view view_ChopLeft(view *v, size_t n);
VIEWPROC view view_ChopRight(view *v, size_t n);
VIEWPROC view view_TrimLeft(view v);
VIEWPROC view view_TrimRight(view v);
VIEWPROC view view_Trim(view v);

#define view_IterateLines(src, idxName, lineName) \
    view lineName = view_ChopByDelim(&src, '\n'); \
    for(size_t idxName = 0; src.Len > 0 || lineName.Len > 0; lineName = view_ChopByDelim(&src, '\n'), idxName++)

#define view_IterateSpaces(src, idxName, wordName) \
    view wordName = view_ChopByAnyDelim(&src, VIEW_STATIC(" \n\t\v\f\r"), 0); \
    for(size_t idxName = 0; src.Len > 0 || wordName.Len > 0; wordName = view_ChopByAnyDelim(&src, VIEW_STATIC(" \n\t\v\f\r"), 0), idxName++) \
        if(word.Len > 0)

#define view_IterateDelimiters(src, delims, idxName, tokName, delimName) \
    char delimName; \
    view tokName = view_ChopByAnyDelim(&src, delims, &delimName); \
    for(size_t idxName = 0; src.Len > 0 || tokName.Len > 0 || delimName != '\0'; \
        tokName = view_ChopByAnyDelim(&src, delims, &delimName), idxName++)

#define PARSE_FAIL 0
#define PARSE_NO_DECIMALS 1 // for when you might want integer precision
#define PARSE_OK 2
VIEWPROC bool view_ParseS64(view v, s64 *Result, view *Remaining);
VIEWPROC int view_ParseF64(view v, f64 *Result, view *Remaining);

typedef struct {
    view File;
    s32 Line;
    s32 Column;
    view Proc;
} code_location;
#define CURR_LOC \
(code_location){VIEW_STATIC(__FILE__), __LINE__, __COLUMN__, VIEW_STATIC(__PROC__)}
// odin style, unix style would be :%d:%d
// prefer LOC_STR to fmt + arg if possible
#define LOC_UNIX_STR __FILE__":"stringify(__LINE__)":"stringify(__COLUMN__)
#define LOC_MSVC_STR __FILE__"("stringify(__LINE__)")"
#define LOC_STR __FILE__"("stringify(__LINE__)":"stringify(__COLUMN__)")"
#define LOC_FMT VIEW_FMT"(%d:%d)"
#define LOC_ARG(loc) VIEW_ARG((loc).File), (loc).Line, (loc.Column)

////////////////////////////////

typedef struct {
    size_t Size;
    u8 *Base;
    size_t Used;

    s32 ScratchCount;
    s32 SplitCount;
} memory_arena;

typedef struct {
    memory_arena *Arena;
    size_t StartMemOffset;
} scratch_arena;

#ifndef ARENAPROC
# define ARENAPROC VLIBPROC
#endif

struct ArenaGetRemaining_opts {
    memory_arena *Arena;
    size_t Alignment;
};
struct ArenaPushSize_opts {
    memory_arena *Arena;
    size_t RequestSize;
    size_t Alignment;
};
struct ArenaSplit_opts {
    memory_arena *Arena;
    memory_arena *SplitArena;
    size_t SplitSize;
};

// NOTE: Thanks Vjekoslav for the idea! (https://twitter.com/vkrajacic/status/1749816169736073295)

#define ArenaGetRemaining(arena, ...) ArenaGetRemaining_Opt((struct ArenaGetRemaining_opts){.Arena = (arena), __VA_ARGS__})
#define ArenaPushSize(arena, size, ...) ArenaPushSize_Opt((struct ArenaPushSize_opts){.Arena = (arena), .RequestSize = (size), __VA_ARGS__})
#define PushStruct(arena, type, ...) ArenaPushSize_Opt((struct ArenaPushSize_opts){.Arena = (arena), .RequestSize = sizeof(type), __VA_ARGS__})
#define PushArray(arena, count, type, ...) ArenaPushSize_Opt((struct ArenaPushSize_opts){.Arena = (arena), .RequestSize = (count)*sizeof(type), __VA_ARGS__})
#define ArenaClear(arena, ZeroMem) if(ZeroMem) {mem_zero((arena)->Base, (arena)->Size);} (arena)->Used = 0
ARENAPROC char *Arena_strndup(memory_arena *Arena, const char *s, size_t n);
// s MUST be null terminated
ARENAPROC char *Arena_strdup(memory_arena *Arena, const char *s);

/* Split arenas work like a stack, you must rejoin them as first in last out.
 * When you call ArenaSplit, it will remove the size requested from the original at (Base + Size - SplitSize)
 * Calling ArenaSplit without SplitSize will split the arena into two equal parts (*they could be different sizes due to alignment)
**/
#define ArenaSplit(arena, split, ...) ArenaSplit_Opt((struct ArenaSplit_opts){.Arena = (arena), .SplitArena = (split), __VA_ARGS__})
// Split an arena into multiple equal parts
#define ArenaSplitMultiple(arena, split, ...) ArenaSplitMultiple_Impl((arena), \
    (memory_arena*[]){(split), __VA_ARGS__}, sizeof((memory_arena*[]){(split), __VA_ARGS__})/sizeof(memory_arena*))
#define ArenaRejoinMultiple(arena, split, ...) ArenaRejoinMultiple_Impl((arena), \
    (memory_arena*[]){(split), __VA_ARGS__}, sizeof((memory_arena*[]){(split), __VA_ARGS__})/sizeof(memory_arena*))
ARENAPROC void ArenaInit(memory_arena *Arena, size_t Size, void *Base);
ARENAPROC scratch_arena ArenaBeginScratch(memory_arena *Arena);
ARENAPROC void ArenaEndScratch(scratch_arena Scratch, bool ZeroMem);
ARENAPROC size_t ArenaGetAlignmentOffset(memory_arena *Arena, size_t Alignment);

ARENAPROC size_t ArenaGetRemaining_Opt(struct ArenaGetRemaining_opts opt);
ARENAPROC void *ArenaPushSize_Opt(struct ArenaPushSize_opts opt);
ARENAPROC void ArenaSplit_Opt(struct ArenaSplit_opts opt);
ARENAPROC void ArenaRejoin(memory_arena *Arena, memory_arena *SplitArena);
ARENAPROC void ArenaSplitMultiple_Impl(memory_arena *Arena, memory_arena **SplitArenas, size_t SplitArenaCount);
ARENAPROC void ArenaRejoinMultiple_Impl(memory_arena *Arena, memory_arena **SplitArenas, size_t SplitArenaCount);

#ifndef VICLIB_NO_TEMP_ARENA
# define temp_reset() ArenaClear(&ArenaTemp, true)
// will align to 4 bytes
# define temp_alloc(size, ...) ArenaPushSize_Opt((struct ArenaPushSize_opts){.Arena = &ArenaTemp, .RequestSize = (size), __VA_ARGS__})
# define temp_strdup(s) Arena_strdup(&ArenaTemp, s)
# define temp_strndup(s, n) Arena_strndup(&ArenaTemp, s, n)
# define temp_save() ArenaTemp.Used
# define temp_rewind(checkpoint) ArenaTemp.Used = checkpoint;
#endif

////////////////////////////////

#ifdef RADDBG_MARKUP_H
#if COMPILER_GCC || COMPILER_CLANG
// temporary solution
PRAGMA(GCC diagnostic push)
PRAGMA(GCC diagnostic ignored "-Wattributes")
#endif
raddbg_type_view(view, array($.Data, $.Len));
#if defined(SDL_h_)
raddbg_type_view(SDL_Surface, $.format == SDL_PixelFormat.SDL_PIXELFORMAT_RGBA32 ? 
    bitmap($.pixels, $.w, $.h, RGBA32) : $);
#endif
#if COMPILER_GCC || COMPILER_CLANG
RESTORE_WARNINGS
#endif

#endif // RADDBG_MARKUP_H

////////////////////////////////

#if !defined(VICLIB_NO_FILE_IO)

#define ERROR_READ_UNKNOWN 1
#define ERROR_READ_FILE_NOT_FOUND 2
#define ERROR_READ_ACCESS_DENIED 3
#define ERROR_READ_NO_MEM 4
#define ERROR_READ_FILE_TOO_BIG 5 // READ_ENTIRE_FILE_MAX exceeded

#define ERROR_WRITE_UNKNOWN 1

#ifndef READ_ENTIRE_FILE_MAX
#define READ_ENTIRE_FILE_MAX 0xFFFFFFFF
#endif

#if defined(_UNISTD_H_) && defined(_SYS_STAT_H_)
# define VL_FILE_LINUX
#endif

#if defined(_APISETFILE_) || defined(VL_FILE_LINUX)
bool GetLastWriteTime(const char *file, u64 *WriteTime);
#else
#define GetLastWriteTime(file, writeTime) \
AssertMsgAlways(false, "To use GetLastWriteTime you must:\n" \
    " - include windows.h or fileapi.h (windows api)\n" \
    " - inlcude sys/stat.h (linux)")
#endif

#if defined(_APISETFILE_) || defined(_INC_STDIO) || defined(VL_FILE_LINUX)

typedef struct {
#if defined(_APISETFILE_)
    HANDLE File;
#elif defined(_INC_STDIO)
    FILE *File;
#elif defined(VL_FILE_LINUX)
    int fd;
    bool didFirstIteration; // Need because fd could be 0 (stdin)
#endif
    u32 BufferSize;
    u8 *Buffer;
    size_t RemainingFileSize;
} file_chunk;

VLIBPROC bool ReadFileChunk(file_chunk *Chunk, const char *File, u32 *ChunkSize);
VLIBPROC bool WriteEntireFile(const char *File, const void *Data, size_t Size);

#if (defined(_APISETFILE_) && defined(_MEMORYAPI_H_)) || (defined(_INC_STDIO) && defined(malloc)) || (defined(VL_FILE_LINUX) && defined(mmap))
char *ReadEntireFile(const char *File, size_t *Size);
#endif

#else

#define ReadEntireFile(File, Size) (void)File; (void)Size; \
AssertMsgAlways(false, "To use 'ReadEntireFile' you must\n:" \
" - include both stdlib.h and stdio.h (stdlib)\n" \
" - either windows.h or fileapi.h (windows api)\n" \
" - include both unistd.h, sys/stat.h and sys/mman.h (linux)")
#define ReadFileChunk(Chunk, File, ChunkSize) /* (void)Chunk; */\
AssertMsgAlways(false, "To use 'ReadFileChunk' you must\n:" \
" - include stdio.h (stdlib)\n" \
" - either windows.h or fileapi.h (windows api)\n" \
" - include unistd.h, sys/stat.h (linux)")
#define WriteEntireFile(File, Data, Size) \
AssertMsgAlways(false, "To use 'ReadFileChunk' you must\n:" \
" - include stdio.h (stdlib)\n" \
" - either windows.h or fileapi.h (windows api)\n" \
" - include unistd.h, sys/stat.h (linux)")

#endif
#endif // !defined(VICLIB_NO_FILE_IO)

#ifndef VICLIB_NO_SORT
bool int_less_than(const void *A, const void *B);

#define VL_Swap(A,B) temp = (A); (A) = (B); (B) = temp
#define VL_SwapType(A,B,T) VL_SwapSize(&(A), &(B), sizeof(T))
void VL_SwapSize(void *A, void *B, size_t Size);
// introsort
void Sort(void *Data, size_t Count, size_t ElementSize, bool (*less_than)(const void *, const void *));
void VL_IntroSort(void *Data, size_t lo, size_t hi, int Depth, size_t ElementSize, bool (*less_than)(const void *, const void *));
void VL_InsertionSort(void *Data, size_t Count, size_t ElementSize, bool (*less_than)(const void *, const void *));
void VL_HeapSort(void *Data, size_t Count, size_t ElementSize, bool (*less_than)(const void *, const void *));

#endif // !defined(VICLIB_NO_SORT)

#ifdef VICLIB_IMPLEMENTATION

int is_space(int _c)
{
    char c = (char)_c;
    return(c == ' ' || c == '\n' || c == '\t' ||
           c == '\v' || c == '\f' || c == '\r');
}

#ifndef strlen
size_t strlen(const char *s)
{
    size_t n = 0;
    for(; *s != 0; s++) n++;
    return n;
}
#endif // strlen

VIEWPROC view view_FromParts(const char *Data, size_t Count)
{
    view v;
    v.Data = Data;
    v.Len = Count;
    return v;
}

VIEWPROC view view_FromCstr(const char *Cstr)
{
    view v;
    v.Data = Cstr;
    v.Len = strlen(Cstr);
    return v;
}

VIEWPROC view view_Slice(view A, size_t Start, size_t End)
{
    AssertMsg(Start <= End, "Start must be smaller or equal to End");
    return view_FromParts(A.Data + Start, End - Start);
}

VIEWPROC view view_TrimLeft(view v)
{
    size_t i = 0;
    for(;i < v.Len && is_space(v.Data[i]);) i += 1;

    return view_FromParts((const char*)(v.Data + i), v.Len - i);
}
VIEWPROC view view_TrimRight(view v)
{
    size_t i = 0;
    for(;i < v.Len && is_space(v.Data[v.Len - 1 - i]);) i += 1;

    return view_FromParts((const char*)v.Data, v.Len - i);
}
VIEWPROC view view_Trim(view v)
{
    return view_TrimRight(view_TrimLeft(v));
}

VIEWPROC int view_Compare(view A, view B)
{
    char res = 0;
    for(size_t i = 0; i < min(A.Len, B.Len); i++)
    {
        res = A.Data[i] - B.Data[i];
        if(res != 0) return res;
    }
    return (int)A.Len - (int)B.Len;
}

VIEWPROC bool view_Eq(view A, view B)
{
    if(A.Len != B.Len) return false;
    else return mem_compare(A.Data, B.Data, A.Len) == 0;
}

VIEWPROC bool view_StartsWith(view v, view Start)
{
    if(Start.Len > v.Len) return false;
    else return view_Eq(view_FromParts(v.Data, Start.Len), Start);
}

VIEWPROC const char *view_Contains(view Haystack, view Needle)
{
    if(Needle.Len > Haystack.Len) return 0;
    else if(Needle.Len == Haystack.Len) {
        return mem_compare(Haystack.Data, Needle.Data, Haystack.Len) == 0 ?
            Haystack.Data : 0;
    }
    
    // TODO: IMPORTANT: Do this with a string-search algorithm which requires memory, but is O(n+m)
    // NOTE: I can probably still do this O(n^2) when working with a small haystack (~5000 characters or less?)
    // NOTE: wiki: https://en.wikipedia.org/wiki/Two-way_string-matching_algorithm
    for(size_t i = 0; i < Haystack.Len - Needle.Len; i++)
    {
        if(mem_compare(Haystack.Data + i, Needle.Data, Needle.Len) == 0) return Haystack.Data + i;
    }
    return 0;
}

VIEWPROC bool view_ContainsCharacter(view v, char c)
{
    for(size_t i = 0; i < v.Len; i++) {
        if(v.Data[i] == c) return true;
    }
    return false;
}

VIEWPROC bool view_EndsWith(view v, view End)
{
    if(End.Len > v.Len) return false;
    else return view_Eq(view_FromParts(v.Data + v.Len - End.Len, End.Len), End);
}

VIEWPROC view view_ChopByDelim(view *v, char Delim)
{
    size_t i = 0;
    for(;i < v->Len && v->Data[i] != Delim;) i += 1;

    view Result = view_FromParts((const char*)v->Data, i);

    if(i < v->Len) {
        v->Len -= i + 1;
        v->Data += i + 1;
    }
    else {
        v->Len -= i;
        v->Data += i;
    }

    return Result;
}

VIEWPROC view view_ChopByAnyDelim(view *v, view Delims, char *Delimiter)
{
    if(Delimiter) *Delimiter = 0;

    size_t i = 0;
    for(; i < v->Len; i++) {
        for(size_t j = 0; j < Delims.Len; j++)
            if(Delims.Data[j] == v->Data[i]) {
                if(Delimiter) *Delimiter = v->Data[i];
                goto done;
            }
    }

done:
    view Result = view_FromParts((const char*)v->Data, i);

    if(i < v->Len) {
        v->Len -= i + 1;
        v->Data += i + 1;
    }
    else {
        v->Len -= i;
        v->Data += i;
    }

    return Result;
}

VIEWPROC view view_ChopByView(view *v, view Delim)
{
    view Window = view_FromParts((const char*)v->Data, Delim.Len);
    size_t i = 0;
    for(;i + Delim.Len < v->Len && !view_Eq(Window, Delim);)
    {
        i++;
        Window.Data++;
    }

    view Result = view_FromParts((const char*)v->Data, i);

    if(i + Delim.Len == v->Len) {
        // include last <Delim.Len> characters if they aren't
        //  equal to Delim
        Result.Len += Delim.Len;
    }

    v->Data += i + Delim.Len;
    v->Len -= i + Delim.Len;

    return Result;
}

VIEWPROC view view_ChopLeft(view *v, size_t n)
{
    if(n > v->Len) n = v->Len;

    view Result = view_FromParts((const char*)v->Data, n);
    v->Data += n;
    v->Len -= n;

    return Result;
}

VIEWPROC view view_ChopRight(view *v, size_t n)
{
    if(n > v->Len) n = v->Len;

    view Result = view_FromParts((const char*)(v->Data + v->Len - n), n);
    v->Len -= n;

    return Result;
}

int _digit_val(int c)
{
    int v = 16;
    if(c >= '0' && c <= '9') {
        v = c - '0';
    }
    else if(c >= 'a' && c <= 'z') {
        v = c - 'a' + 10;
    }
    else if(c >= 'A' && c <= 'Z') {
        v = c - 'A' + 10;
    }
    return v;
}

// '0' is the prefix for octal in C, surprisingly, so '080' is an invalid number
// try to compile: int n = 080;
// I checked out how Odin did octal and they do "0o" prefix, seemed more logical.
// I also don't know what the "0z" prefix of base 12 is for but I'll just leave it there
VIEWPROC bool view_ParseS64(view v, s64 *Result, view *Remaining)
{
    AssertMsg(Result != 0, "Result parameter must be a valid pointer");
    v = view_TrimLeft(v);
    if(v.Len == 0) return false;

    bool Neg = false;
    if(v.Len > 1) {
        if(v.Data[0] == '-') {
            Neg = true;
            v.Data++;
            v.Len--;
        }
        else if(v.Data[0] == '+') {
            v.Data++;
            v.Len--;
        }
    }

    int Base = 10;
    // try to parse a prefix
    if(v.Len > 2 && v.Data[0] == '0') {
        if(v.Data[1] == 'b' || v.Data[1] == 'B') {
            Base = 2; v.Data += 2; v.Len -= 2;
        }
        else if(v.Data[1] == 'o') {
            Base = 8; v.Data += 2; v.Len -= 2;
        }
        else if(v.Data[1] == 'd') {
            Base = 10; v.Data += 2; v.Len -= 2;
        }
        else if(v.Data[1] == 'z') {
            Base = 12; v.Data += 2; v.Len -= 2;
        }
        else if(v.Data[1] == 'x' || v.Data[1] == 'X') {
            Base = 16; v.Data += 2; v.Len -= 2;
        }
    }

    int Digit = _digit_val((int)v.Data[0]);
    if(Digit >= Base) return false;

    s64 Value = 0;
    int j = 0;
    for(; j < (int)v.Len; j++)
    {
        char c = v.Data[j];
        if(c != '_') {
            Digit = _digit_val((int)c);
            if(Digit >= Base) {
                // invalid digit
                break;
            }
            Value *= Base;
            Value += Digit;
        }
    }

    if(Neg) Value = -Value;
    *Result = Value;
    if(Remaining) {
        Remaining->Data = v.Data + (size_t)j;
        Remaining->Len = v.Len - (size_t)j;
    }

    return true;
}

int view_ParseF64(view v, f64 *Result, view *Remaining)
{
    AssertMsg(Result != 0, "Result parameter must be a valid pointer");
    v = view_TrimLeft(v);
    if(v.Len == 0) return PARSE_FAIL;
    bool DecimalPart = false;

    bool Neg = false;
    if(v.Len > 1) {
        if(v.Data[0] == '-') {
            Neg = true;
            v.Data++;
            v.Len--;
        }
        else if(v.Data[0] == '+') {
            v.Data++;
            v.Len--;
        }
    }

    if(v.Len > 1 && v.Data[0] == '.') {
        DecimalPart = true;
        v.Data++;
        v.Len--;
    }

    if(*v.Data > '9' || *v.Data < '0') return PARSE_FAIL;

    double Val = 0.0;
    double Multiplier = 1.0;

    bool FoundExponent = false;
    view ExponentStr;
    size_t j = 0;
    for(; j < v.Len; j++)
    {
        char c = v.Data[j];
        if(c == '.') {
            DecimalPart = true;
            if(j+1 < v.Len && (v.Data[j+1] > '9' || v.Data[j+1] < '0')) {
                break;
            }
        }
        else if(c == 'e' || c == 'E') {
            // Try to parse exponent
            // Not sure how I feel about this, there probably is a better way to do this
            ExponentStr = view_FromParts(v.Data + j + 1, v.Len - j - 1);

            bool Prefixed = false;
            bool ExpNeg = false;
            if(ExponentStr.Len > 1) {
                if(ExponentStr.Data[0] == '-') {
                    Prefixed = true;
                    ExpNeg = true;
                    ExponentStr.Data++;
                    ExponentStr.Len--;
                }
                else if(ExponentStr.Data[0] == '+') {
                    Prefixed = true;
                    ExponentStr.Data++;
                    ExponentStr.Len--;
                }
            }

            if(ExponentStr.Len < 1) break;
            if(*ExponentStr.Data > '9' || *ExponentStr.Data < '0') break;
            
            FoundExponent = true;
            size_t ExpEnd = 0;
            for(; ExpEnd < ExponentStr.Len; ExpEnd++)
            {
                if(ExponentStr.Data[ExpEnd] > '9' || ExponentStr.Data[ExpEnd] < '0') break;
            }

            double ExpMultiplier = 10.0;
            int charVal = ExponentStr.Data[ExpEnd - 1] - '0';
            if(ExpNeg) {
                for(int k = 0; k < charVal; k++) Val /= ExpMultiplier;
                ExpMultiplier = 10000000000.0;
                for(s64 k = (s64)ExpEnd - 2; k >= 0; k--)
                {
                    charVal = ExponentStr.Data[k] - '0';
                    for(int l = 0; l < charVal; l++) Val /= ExpMultiplier;
                    ExpMultiplier *= 10000000000.0;
                }
            } else {
                for(int k = 0; k < charVal; k++) Val *= ExpMultiplier;
                ExpMultiplier = 10000000000.0;
                for(s64 k = (s64)ExpEnd - 2; k >= 0; k--)
                {
                    charVal = ExponentStr.Data[k] - '0';
                    for(int l = 0; l < charVal; l++) Val *= ExpMultiplier;
                    ExpMultiplier *= 10000000000.0;
                }
            }

            j += 1 + ExpEnd + (size_t)Prefixed;
            break;
        }
        else if(c > '9' || c < '0') {
            // found non numeric character
            break;
        }
        else if(DecimalPart) {
            Multiplier *= 0.1;
            Val += (v.Data[j] - '0')*Multiplier;
        }
        else {
            Val *= 10.0;
            Val += v.Data[j] - '0';
        }
    }

    if(Neg) Val = -Val;
    *Result = Val;
    if(Remaining) {
        Remaining->Data = v.Data + j;
        Remaining->Len = v.Len - j;
    }
    return (DecimalPart || FoundExponent) ? PARSE_OK : PARSE_NO_DECIMALS;
}

////////////////////////////////

#if !defined(_INC_STRING)
#define VCL_ALIGN (sizeof(size_t)-1)
VLIBPROC void mem_copy_non_overlapping(void *dst, const void *src, size_t len)
{
    u8 *d = (u8*)dst;
    const u8 *s = (const u8*)src;
    if(((uintptr_t)d & VCL_ALIGN) != ((uintptr_t)s & VCL_ALIGN))
        goto misaligned;

    for(; ((uintptr_t)d & VCL_ALIGN) && len != 0; len--) *d++ = *s++;
    if(len != 0) {
        size_t *wideDst = (size_t*)d;
        const size_t *wideSrc = (const size_t*)s;

        for(; len >= sizeof(size_t); len -= sizeof(size_t)) *wideDst++ = *wideSrc++;
        d = (u8*)wideDst;
        s = (const u8*)wideSrc;
misaligned:
        for(; len != 0; len--) *d++ = *s++;
    }
}
#undef VCL_ALIGN

VLIBPROC void mem_copy(void *dst, const void *src, size_t len)
{
    u8 *d = (u8*)dst;
    const u8 *s = (const u8*)src;

    if((uintptr_t)dst == (uintptr_t)src) return;
    if((uintptr_t)s-(uintptr_t)d-len <= -2*len) {
        mem_copy_non_overlapping(d, s, len);
        return;
    }

    if(d < s) {
#if COMPILER_GCC
		if((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
			while((uintptr_t)d % sizeof(size_t)) {
				if(!len--) return;
				*d++ = *s++;
			}
            __attribute__((__may_alias__)) size_t *wideDst = (size_t*)d;
            __attribute__((__may_alias__)) size_t *wideSrc = (size_t*)s;
			for(; len >= sizeof(size_t); len -= sizeof(size_t)) *d++ = *s++;
            d = (u8*)wideDst;
            s = (const u8*)wideSrc;
        }
#endif
		for (; len; len--) *d++ = *s++;
    } else {
#if COMPILER_GCC
		if((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
			while((uintptr_t)(d + len) % sizeof(size_t)) {
				if(!len--) return;
				d[len] = s[len];
			}
			while(len >= sizeof(size_t)) {
                len -= sizeof(size_t);
                *(__attribute__((__may_alias__)) size_t*)(d + len) = 
                    *(__attribute__((__may_alias__)) size_t*)(s + len);
            }
        }
#endif
		while(len) len--, d[len] = s[len];
    }
}

VLIBPROC void mem_zero(void *data, size_t len)
{
    size_t i;

    if ((uintptr_t)data % sizeof(size_t) == 0 &&
        len % sizeof(size_t) == 0) {
        size_t *d = (size_t*)data;

        for(i = 0; i < len/sizeof(size_t); i++) {
            d[i] = 0;
        }
    }
    else {
        char *d = (char*)data;

        for(i = 0; i < len; i++) {
            d[i] = 0;
        }
    }
}

VLIBPROC int mem_compare(const void *str1, const void *str2, size_t count)
{
    register const unsigned char *s1 = (const unsigned char*)str1;
    register const unsigned char *s2 = (const unsigned char*)str2;

    for(;count-- > 0;)
    {
        if(*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}
#endif // !defined(_INC_STRING)

////////////////////////////////

#ifndef VICLIB_NO_TEMP_ARENA
# ifndef VICLIB_TEMP_SIZE
#  define VICLIB_TEMP_SIZE (4*1024*1024)
# endif // !defined(VICLIB_TEMP_SIZE)
static u8 ViclibTempMem[VICLIB_TEMP_SIZE] = {0};
memory_arena ArenaTemp = {
    .Size = VICLIB_TEMP_SIZE,
    .Base = ViclibTempMem,
    .Used = 0,
    .ScratchCount = 0,
};
#endif // !defined(VICLIB_NO_TEMP_ARENA)

ARENAPROC char *Arena_strndup(memory_arena *Arena, const char *s, size_t n)
{
    char *Result = ArenaPushSize(Arena, n + 1);
    mem_copy_non_overlapping(Result, s, n);
    Result[n] = '\0';
    return Result;
}

ARENAPROC char *Arena_strdup(memory_arena *Arena, const char *s)
{
    size_t n = strlen(s);
    return Arena_strndup(Arena, s, n);
}

ARENAPROC void ArenaInit(memory_arena *Arena, size_t Size, void *Base)
{
    Arena->Used = 0;
    Arena->Size = Size;
    Arena->Base = (u8*)Base;
    Arena->ScratchCount = 0;
}

ARENAPROC size_t ArenaGetAlignmentOffset(memory_arena *Arena, size_t Alignment)
{
    size_t AlignOffset = 0;
    size_t CurrentMemLoc = (size_t)Arena->Base + Arena->Used;
    size_t AlignMask = Alignment - 1;
    if(CurrentMemLoc & AlignMask) {
        AlignOffset = Alignment - (CurrentMemLoc & AlignMask);
    }
    return AlignOffset;
}

ARENAPROC size_t ArenaGetRemaining_Opt(struct ArenaGetRemaining_opts opt)
{
    if(opt.Alignment < 1) opt.Alignment = 4;
    size_t Result = opt.Arena->Size - (opt.Arena->Used + ArenaGetAlignmentOffset(opt.Arena, opt.Alignment));
    return Result;
}

ARENAPROC void *ArenaPushSize_Opt(struct ArenaPushSize_opts opt)
{
    if(opt.Alignment < 1) opt.Alignment = 4;
    size_t Size = opt.RequestSize;
    size_t AlignOffset = ArenaGetAlignmentOffset(opt.Arena, opt.Alignment);
    Size += AlignOffset;

    AssertMsg((opt.Arena->Used + Size) <= opt.Arena->Size, "Assert Fail: Full arena size reached");
    void *Mem = opt.Arena->Base + opt.Arena->Used + AlignOffset;
    opt.Arena->Used += Size;

    return Mem;
}

ARENAPROC void ArenaSplit_Opt(struct ArenaSplit_opts opt)
{
    AssertMsg(opt.Arena->Size > opt.SplitSize, "Need more memory in arena to split to requested size");
    if(opt.SplitSize == 0) opt.SplitSize = ArenaGetRemaining(opt.Arena, .Alignment = 1) / 2;

    opt.Arena->SplitCount++;
    opt.Arena->Size = opt.Arena->Size - opt.SplitSize;

    opt.SplitArena->Size = opt.SplitSize;
    opt.SplitArena->Base = opt.Arena->Base + opt.Arena->Size;
    opt.SplitArena->Used = 0;
    opt.SplitArena->ScratchCount = 0;
    opt.SplitArena->SplitCount = 0;
    ArenaPushSize(opt.SplitArena, 0, .Alignment = 4); // 'leak' up to 4 bytes here to keep the memory aligned
}

ARENAPROC void ArenaRejoin(memory_arena *Arena, memory_arena *SplitArena)
{
    AssertMsg(Arena->SplitCount > 0, "This split arena doesn't belong to the arena");
    AssertMsg(Arena->Base + Arena->Size == SplitArena->Base, "Split arenas must be rejoined as a stack; first in last out");
    AssertMsg(SplitArena->ScratchCount == 0, "Rejoining a split arena without Ending all its scratch arenas will cause conflicts with the original arena");
    AssertMsg(SplitArena->SplitCount == 0, "Rejoining a split arena without rejoining its split arenas will cause memory leaks");
    SplitArena->Base = 0;
    SplitArena->Used = 0;
    Arena->Size += SplitArena->Size;
    Arena->SplitCount--;
}

ARENAPROC void ArenaSplitMultiple_Impl(memory_arena *Arena, memory_arena **SplitArenas, size_t SplitArenaCount)
{
    size_t SplitSize = ArenaGetRemaining(Arena, .Alignment = 1) / (SplitArenaCount + 1);
    for(size_t SplitIdx = 0; SplitIdx < SplitArenaCount; SplitIdx++) ArenaSplit(Arena, SplitArenas[SplitIdx], SplitSize);
}

ARENAPROC void ArenaRejoinMultiple_Impl(memory_arena *Arena, memory_arena **SplitArenas, size_t SplitArenaCount)
{
    for(int64_t SplitIdx = (int64_t)SplitArenaCount - 1; SplitIdx >= 0; SplitIdx--) ArenaRejoin(Arena, SplitArenas[SplitIdx]);
}

ARENAPROC scratch_arena ArenaBeginScratch(memory_arena *Arena)
{
    scratch_arena Scratch = {
        .Arena = Arena,
        .StartMemOffset = Arena->Used,
    };
    Arena->ScratchCount += 1;

    return Scratch;
}

ARENAPROC void ArenaEndScratch(scratch_arena Scratch, bool ZeroMem)
{
    memory_arena *Arena = Scratch.Arena;
    Assert(Arena->Used >= Scratch.StartMemOffset);
    if(ZeroMem) {
        mem_zero(Arena->Base + Scratch.StartMemOffset, Arena->Used - Scratch.StartMemOffset);
    }
    Arena->Used = Scratch.StartMemOffset;
    Assert(Arena->ScratchCount > 0);
    Arena->ScratchCount -= 1;
}

////////////////////////////////

#if defined(_APISETFILE_) // windows fileapi.h included
bool GetLastWriteTime(const char *file, u64 *WriteTime)
{
    bool ok = false;
    WIN32_FILE_ATTRIBUTE_DATA data;
    if(GetFileAttributesEx(file, GetFileExInfoStandard, &data) &&
        (uint64_t)data.nFileSizeHigh + (uint64_t)data.nFileSizeLow > 0)
    {
        *WriteTime = *(uint64_t*)&data.ftLastWriteTime;
        ok = true;
    }
    return ok;
}
#elif defined(_SYS_STAT_H_)
bool GetLastWriteTime(const char *file, uint64_t *WriteTime)
{
    struct stat attr;
    bool ok = false;
    if(!stat(file, &attr) && attr.st_size > 0) {
        ok = true;
        *WriteTime = *(uint64_t*)&attr.st_mtim;
    }
    return ok;
}
#endif

#if !defined(VICLIB_NO_FILE_IO)
#if defined(_APISETFILE_) // windows fileapi.h included

#if defined(_MEMORYAPI_H_)
PUSH_IGNORE_UNINITIALIZED
char *ReadEntireFile(const char *File, size_t *Size)
{
    ErrorNumber = 0;
    AssertMsg(Size != 0, "Size parameter must be a valid pointer");
    char *Result = 0;
    HANDLE FileHandle = CreateFileA(File, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    bool ok = FileHandle != INVALID_HANDLE_VALUE;
    if(!ok) {
        DWORD Error = GetLastError();
        switch(Error) {
            case ERROR_INVALID_DRIVE: fallthrough;
            case ERROR_PATH_NOT_FOUND: fallthrough;
            case ERROR_FILE_NOT_FOUND: ErrorNumber = ERROR_READ_FILE_NOT_FOUND; break;
            case ERROR_ACCESS_DENIED: ErrorNumber = ERROR_READ_ACCESS_DENIED; break;

            default: ErrorNumber = ERROR_READ_UNKNOWN; break;
        }
    }

    LARGE_INTEGER FileSize;
    if(ok) {
        ok = (bool)GetFileSizeEx(FileHandle, &FileSize);
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }

    u64 Limit = READ_ENTIRE_FILE_MAX;

    if(ok) {
        ok = (u64)FileSize.QuadPart <= Limit;
        if(!ok) ErrorNumber = ERROR_READ_FILE_TOO_BIG;
    }
    u32 FileSizeU32 = (u32)FileSize.QuadPart;

    if(ok) {
        // TODO: VirtualAlloc or HeapAlloc here?
        Result = (char*)VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        ok = Result != 0;
        if(!ok) ErrorNumber = ERROR_READ_NO_MEM;
    }

    if(ok) {
        DWORD BytesRead;
        if(ReadFile(FileHandle, Result, FileSizeU32, &BytesRead, 0) && (FileSize.QuadPart == BytesRead))
        {
            *Size = FileSize.QuadPart;
        }
        else {
            ErrorNumber = ERROR_READ_UNKNOWN;
            VirtualFree((void*)Result, 0, MEM_RELEASE); // NOTE: If I decide to do HeapAlloc above, this should be HeapFree
            Result = 0;
        }
    }

    CloseHandle(FileHandle);
    return Result;
}
RESTORE_WARNINGS
#endif // defined(_MEMORYAPI_H_) (VirtualAlloc)

bool ReadFileChunk(file_chunk *Chunk, const char *File, u32 *ChunkSize)
{
    AssertMsg(Chunk->Buffer && Chunk->BufferSize, "Requires a valid buffer and a buffer size");
    ErrorNumber = 0;

    if(!Chunk->File) {
        Chunk->File = CreateFileA(File, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if(Chunk->File == INVALID_HANDLE_VALUE) {
            DWORD Error = GetLastError();
            switch(Error) {
                case ERROR_INVALID_DRIVE: fallthrough;
                case ERROR_PATH_NOT_FOUND: fallthrough;
                case ERROR_FILE_NOT_FOUND: ErrorNumber = ERROR_READ_FILE_NOT_FOUND; break;
                case ERROR_ACCESS_DENIED: ErrorNumber = ERROR_READ_ACCESS_DENIED; break;

                default: ErrorNumber = ERROR_READ_UNKNOWN; break;
            }
            return false;
        }

        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(Chunk->File, &FileSize) == 0) {
            ErrorNumber = ERROR_READ_UNKNOWN; // TODO: find out why it failed
            return false;
        }

        Chunk->RemainingFileSize = FileSize.QuadPart;
    } else if(Chunk->RemainingFileSize == 0) {
        CloseHandle(Chunk->File);
        Chunk->File = 0;
        return false;
    }
    size_t OutSize = min(Chunk->RemainingFileSize, (size_t)Chunk->BufferSize);

    DWORD BytesRead;
    if(ReadFile(Chunk->File, Chunk->Buffer, (u32)OutSize, &BytesRead, 0) && (OutSize == BytesRead))
    {
        *ChunkSize = (u32)OutSize;
    } else {
        ErrorNumber = ERROR_READ_UNKNOWN;
        return false;
    }

    Chunk->RemainingFileSize -= (u32)OutSize;
    return true;
}

#elif defined(VL_FILE_LINUX)

#if defined(mmap)
char *ReadEntireFile(char *File, size_t *Size)
{
    ErrorNumber = 0;
    int fd = open(File, O_RDONLY);
    if(fd == -1) {
        if(errno == EACCES || errno == EPERM) ErrorNumber = ERROR_READ_ACCESS_DENIED;
        else if(errno == ENOMEM) ErrorNumber = ERROR_READ_NO_MEM;
        else if(errno == EOVERFLOW) ErrorNumber = ERROR_READ_FILE_TOO_BIG;
        else if(errno == EBADF || errno == ENOENT)
            ErrorNumber = ERROR_READ_FILE_NOT_FOUND;
        else ErrorNumber = ERROR_READ_UNKNOWN;
        return 0;
    }

    struct stat stat;
    if(fstat(fd, &stat) == -1) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        close(fd);
        return 0;
    }

    char *Result = (char*)mmap(0, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED);
    if(Result == MAP_FAILED) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        close(fd);
        return 0;
    }

    ssize_t bytesRead = read(fd, Result, stat.st_size);
    if(bytesRead != stat.st_size) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        close(fd);
        return 0;
    }

    close(fd);

    if(Size) *Size = stat.st_size;

    return Result;
}
#endif // defined(mmap)

bool ReadFileChunk(file_chunk *Chunk, const char *File, u32 *ChunkSize)
{
    AssertMsg(Chunk->Buffer && Chunk->BufferSize, "Requires a valid buffer and a buffer size");
    ErrorNumber = 0;

    if(!Chunk->didFirstIteration) {
        Chunk->didFirstIteration = true;
        Chunk->fd = open(File, O_RDONLY);
        if(Chunk->fd == -1) {
            if(errno == EACCES || errno == EPERM) ErrorNumber = ERROR_READ_ACCESS_DENIED;
            else if(errno == ENOMEM) ErrorNumber = ERROR_READ_NO_MEM;
            else if(errno == EOVERFLOW) ErrorNumber = ERROR_READ_FILE_TOO_BIG;
            else if(errno == EBADF || errno == ENOENT)
                ErrorNumber = ERROR_READ_FILE_NOT_FOUND;
            else ErrorNumber = ERROR_READ_UNKNOWN;
            return false;
        }

        struct stat stat;
        if(fstat(fd, &stat) == -1) {
            ErrorNumber = ERROR_READ_UNKNOWN;
            close(Chunk->fd);
            return false;
        }

        Chunk->RemainingFileSize = stat.st_size;
    } else if(Chunk->RemainingFileSize == 0) {
        if(Chunk->fd > 2) close(Chunk->fd); // I don't think I should try to close stdin, stdout, stderr?
        Chunk->didFirstIteration = false;
        Chunk->fd = 0;
        return false;
    }

    size_t OutSize = min(Chunk->RemainingFileSize, (size_t)Chunk->BufferSize);

    if(OutSize == read(Chunk->fd, Chunk->Buffer, OutSize)) {
        *ChunkSize = (u32)OutSize;
    } else {
        ErrorNumber = ERROR_READ_UNKNOWN;
        return false;
    }

    Chunk->RemainingFileSize -= (u32)OutSize;
    return true;
}

#elif defined(_INC_STDIO)

#if defined(malloc)
PUSH_IGNORE_UNINITIALIZED
char *ReadEntireFile(char *File, size_t *Size)
{
    ErrorNumber = 0;
    // ERROR_READ_UNKNOWN - fseek, ftell failed
    // fopen could fail due to many things
    // TODO: Set ErrorNumber depending on them
    // ERROR_READ_NO_MEM - malloc failed
    // ERROR_READ_FILE_TOO_BIG - READ_ENTIRE_FILE_MAX exceeded
    FILE *f = fopen(File, "rb");
    bool ok = f != 0;
    if(!ok) ErrorNumber = ERROR_READ_FILE_NOT_FOUND;

    if(ok) {
        ok = fseek(f, 0, SEEK_END) >= 0;
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }

    long m;
    if(ok) {
        m = ftell(f);
        ok = (u64)m <= (u64)READ_ENTIRE_FILE_MAX;
        if(!ok) ErrorNumber = ERROR_READ_FILE_TOO_BIG;

        ok = m >= 0;
        // ftell() fail
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }

    if(ok) {
        Result = malloc(sizeof(char) * m);
        ok = Result != 0;
        if(!ok) ErrorNumber = ERROR_READ_NO_MEM;
    }

    if(ok) {
        ok = fseek(f, 0, SEEK_SET) >= 0;
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }

    if(ok) {
        size_t n = fread(Result, 1, m, f);
        ok = n == (size_t)m;
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }

    if(ok) {
        ok = !ferror(f);
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }

    if(Size) *Size = m;

    if(f) fclose(f);
    if(!ok && Result) {
        free(Result);
        Result = 0;
    }

    return Result;
}
RESTORE_WARNINGS
#endif // defined(malloc)

bool ReadFileChunk(file_chunk *Chunk, const char *File, u32 *ChunkSize)
{
    AssertMsg(Chunk->Buffer && Chunk->BufferSize, "Requires a valid buffer and a buffer size");
    ErrorNumber = 0;

    if(!Chunk->File) {
        Chunk->File = fopen(File, "rb");
        if(!Chunk->File) {
            ErrorNumber = ERROR_READ_FILE_NOT_FOUND;
            return false;
        }

        if(fseek(Chunk->File, 0, SEEK_END) < 0) {
            ErrorNumber = ERROR_READ_UNKNOWN;
            return false;
        }

        long m = ftell(Chunk->File);
        if(m < 0) {
            ErrorNumber = ERROR_READ_UNKNOWN;
            return false;
        }

        if(fseek(Chunk->File, 0, SEEK_SET) < 0) {
            ErrorNumber = ERROR_READ_UNKNOWN;
            return false;
        }

        Chunk->RemainingFileSize = m;
    } else if(Chunk->RemainingFileSize == 0) {
        // Success
        fclose(Chunk->File);
        Chunk->File = 0;
        return false;
    }
    size_t OutSize = min(Chunk->RemainingFileSize, (size_t)Chunk->BufferSize);

    if(OutSize == fread(Chunk->Buffer, 1, (u32)OutSize, Chunk->File))
    {
        *ChunkSize = (u32)OutSize;
    } else {
        ErrorNumber = ERROR_READ_UNKNOWN;
        return false;
    }

    Chunk->RemainingFileSize -= (u32)OutSize;
    return true;
}

// TODO: WriteEntire file for windows & linux
VLIBPROC bool WriteEntireFile(const char *File, const void *Data, size_t Size)
{
    const char *buf = 0;
    FILE *f = fopen(File, "wb");
    if(f == 0) {
        ErrorNumber = ERROR_WRITE_UNKNOWN; // TODO: Check errors to see cause
        return false;
    }

    //           len
    //           v
    // aaaaaaaaaa
    //     ^
    //     data

    buf = (const char*)Data;
    while(Size > 0) {
        size_t n = fwrite(buf, 1, Size, f);
        if(ferror(f)) {
            fclose(f);
            ErrorNumber = ERROR_WRITE_UNKNOWN; // TODO: Check errors to see cause
            return false;
        }
        Size -= n;
        buf  += n;
    }

    return true;
}

#endif // stdio.h included
#endif // !defined(VICLIB_NO_FILE_IO)

#ifndef VICLIB_NO_SORT

bool int_less_than(const void *A, const void *B) {
    return (*(int*)A < *(int*)B);
}

void VL_SwapSize(void *A, void *B, size_t Size)
{
    size_t i;
    if((uintptr_t)A % sizeof(size_t) == 0 &&
       (uintptr_t)B % sizeof(size_t) == 0 &&
       Size % sizeof(size_t) == 0)
    {
        size_t temp;
        size_t *APtr = (size_t*)A;
        size_t *BPtr = (size_t*)B;

        for(i = 0; i < Size/sizeof(size_t); i++) {
            VL_Swap(*APtr, *BPtr);
            APtr++; BPtr++;
        }
    }
    else {
        char temp;
        char *APtr = (char*)A;
        char *BPtr = (char*)B;

        for(i = 0; i < Size; i++) {
            VL_Swap(*APtr, *BPtr);
            APtr++; BPtr++;
        }
    }
}

// introsort
void Sort(void *Data, size_t Count, size_t ElementSize, bool (*less_than)(const void *, const void *))
{
    // compute MaxDepth = 2*log_2(Count)
    int MaxDepth = -2;
    for(size_t i = Count; i != 0; i >>= 1) MaxDepth += 2;
    VL_IntroSort(Data, 0, Count - 1, MaxDepth, ElementSize, less_than);
}
void VL_IntroSort(void *Data, size_t lo, size_t hi, int Depth, size_t ElementSize, bool (*less_than)(const void *, const void *))
{
    if(lo < hi) {
        if((hi - lo + 1) < 16) {
            VL_InsertionSort((u8*)Data + lo*ElementSize, hi - lo + 1, ElementSize, less_than);
        }
        else if(Depth == 0) {
            VL_HeapSort((u8*)Data + lo*ElementSize, hi - lo + 1, ElementSize, less_than);
        }
        else {
            size_t PivotIdx = (lo + hi)/2; // median of 3 could be better here, I'm not sure

            VL_SwapSize((u8*)Data + PivotIdx*ElementSize, (u8*)Data + lo*ElementSize, ElementSize);
            void *Pivot = (u8*)Data + lo*ElementSize;

            size_t i = lo + 1;
            for(size_t j = lo + 1; j < hi + 1; j++)
            {
                if(less_than((u8*)Data + j*ElementSize, Pivot)) {
                    // swap(Arr[i], Arr[j]);
                    VL_SwapSize((u8*)Data + i*ElementSize, (u8*)Data + j*ElementSize, ElementSize);
                    i++;
                }
            }
            // swap(Arr[lo], Arr[i-1]);
            VL_SwapSize((u8*)Data + lo*ElementSize, (u8*)Data + (i-1)*ElementSize, ElementSize);
            PivotIdx = i - 1;

            if(PivotIdx > 0) VL_IntroSort(Data, lo, PivotIdx - 1, Depth - 1, ElementSize, less_than);
            VL_IntroSort(Data, PivotIdx + 1, hi, Depth - 1, ElementSize, less_than);
        }
    }
}

void VL_InsertionSort(void *Data, size_t Count, size_t ElementSize, bool (*less_than)(const void *, const void *))
{
    for(size_t i = 1; i < Count; i++)
    {
        void *Val = (u8*)Data + i*ElementSize;

        int64_t j = i - 1;
        for(;j >= 0 && less_than(Val, (u8*)Data + j*ElementSize); j--)
        {
            mem_copy((u8*)Data + (j+1)*ElementSize, (u8*)Data + j*ElementSize, ElementSize);
        }
        mem_copy((u8*)Data + (j+1)*ElementSize, Val, ElementSize);
    }
}
void VL_HeapSort(void *Data, size_t Count, size_t ElementSize, bool (*less_than)(const void *, const void *))
{
    // left child = 2*i + 1
    // right child = 2*i + 2
    // parent = (i-1)/2 (truncated)

    for(size_t Start = (Count - 2) / 2; Start > 0; Start--) {
        size_t Root = Start;
        while(2*Root + 1 < Count) {
            size_t Child = 2*Root + 1;
            if(Child + 1 < Count && less_than((u8*)Data + Child*ElementSize, (u8*)Data + (Child+1)*ElementSize)) {
                Child++;
            }
            
            if(less_than((u8*)Data + Root*ElementSize, (u8*)Data + Child*ElementSize)) {
                VL_SwapSize((u8*)Data + Root*ElementSize, (u8*)Data + Child*ElementSize, ElementSize);
                Root = Child;
            } else {
                break;
            }
        }
    }
    
    for(size_t End = Count - 1; End > 0; End--) {
        VL_SwapSize(Data, (u8*)Data + End*ElementSize, ElementSize);
        
        size_t Root = 0;
        while(2*Root + 1 < End) {
            size_t Child = 2*Root + 1;
            if(Child + 1 < End && less_than((u8*)Data + Child*ElementSize, (u8*)Data + (Child+1)*ElementSize)) {
                Child++;
            }
            
            if(less_than((u8*)Data + Root*ElementSize, (u8*)Data + Child*ElementSize)) {
                VL_SwapSize((u8*)Data + Root*ElementSize, (u8*)Data + Child*ElementSize, ElementSize);
                Root = Child;
            } else {
                break;
            }
        }
    }
}
#endif // !defined(VICLIB_NO_SORT)
#endif // VICLIB_IMPLEMENTATION
#endif //VICLIB_H
