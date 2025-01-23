/* date = December 29th 2024 10:12 pm
--Author: Víctor López Cortés
--Usage:
-Defines: To have any of these take effect, you must define them _before_ including this file
BASE_TYPES_IMPLEMENTATION if you want to have the implementation
READ_ENTIRE_FILE_MAX if you want to have a max file read size
QUIET_ASSERT if you want the assertions to add a breakpoint but not print
RELEASE_MODE to have some stuff work faster, right now, assertions get compiled out when this is defined
-Check ErrorNumber when errors occur.

--Many thanks to the inspirations for this library:
- Mr4th's 4ed_base_types.h - https://mr-4th.itch.io/4coder (find the file in 'custom' directory)
- stb header-only libraries - https://github.com/nothings/stb
 - tsoding's string view implementation - https://github.com/tsoding/sv

I modified tsoding's string view so that it doesn't use the stdlib by implementing the couple
functions it uses

LICENCES:
tsoding string view implementation:
// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
# if !defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL
static_assert(0, "\nPlease use the compatible preprocessor for msvc\n"
              "To use it, pass /Zc:preprocessor to msvc\n");
#  error (This error is so the error messages stop here)
# endif
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
 - thread_local
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

/* DebugBreak for different platforms.
Implementation by SDL3 (sdl wiki says SDL2 also had this but code says since SDL3.1.3) */
#if defined(_MSC_VER)
/* Don't include intrin.h here because it contains C++ code */
extern void __cdecl __debugbreak(void);
# define DebugBreak __debugbreak()
#elif defined(ANDROID)
# include <assert.h>
# define DebugBreak assert(0)
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
# define DebugBreak __asm__ __volatile__ ( "int $3\n\t" )
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__riscv)
# define DebugBreak __asm__ __volatile__ ( "ebreak\n\t" )
#elif (defined(OS_MAC) && (defined(__arm64__) || defined(__aarch64__)) )  /* this might work on other ARM targets, but this is a known quantity... */
# define DebugBreak __asm__ __volatile__ ( "brk #22\n\t" )
#elif defined(OS_MAC) && defined(__arm__)
# define DebugBreak __asm__ __volatile__ ( "bkpt #22\n\t" )
#elif defined(_WIN32) && ((defined(__GNUC__) || defined(__clang__)) && (defined(__arm64__) || defined(__aarch64__)) )
# define DebugBreak __asm__ __volatile__ ( "brk #0xF000\n\t" )
#elif defined(__386__) && defined(__WATCOMC__)
# define DebugBreak { _asm { int 0x03 } }
#elif defined(HAVE_SIGNAL_H) && !defined(__WATCOMC__)
# include <signal.h>
# define DebugBreak raise(SIGTRAP)
#else
/* How do we trigger breakpoints on this platform? */
# define DebugBreak Assert(false)
#endif

#if COMPILER_GCC || COMPILER_CLANG
// gcc and cl (haven't tried clang
// don't understand how functions like win32_ReadEntireFile work
// but I want to keep the warnings for other functions
# define PUSH_IGNORE_UNINITIALIZED \
PRAGMA(GCC diagnostic push) \
PRAGMA(GCC diagnostic ignored "-Wuninitialized")
# define RESTORE_WARNINGS PRAGMA(GCC diagnostic pop)
#elif COMPILER_CL
# define PUSH_IGNORE_UNINITIALIZED \
PRAGMA(warning(push)) \
PRAGMA(warning(disable: 4701)) \
PRAGMA(warning(disable: 4703))
# define RESTORE_WARNINGS PRAGMA(warning(pop))
#else
// compiler specific implementation
# define PUSH_IGNORE_UNINITIALIZED
# define RESTORE_WARNINGS
#endif

// only works with static arrays!
#define ArrayLen(arr) sizeof(arr)/sizeof(arr[0])

#define stringify_(a) #a
#define stringify(a) stringify_(a)

#define fallthrough 

// Thanks Martins for the help with this!
// If one parameter was passed, it will select the first
#define SELECT_PROC_1DEFAULT_(_1, _2, NAME, ...) NAME
#define SELECT_PROC_1DEFAULT(A, B, ...) SELECT_PROC_1DEFAULT_(dummy, ##__VA_ARGS__, A, B)

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint8_t  bool;
typedef uint32_t b32;
typedef float f32;
typedef double f64;

#define true 1
#define false 0

// TODO: Print for different platforms
#if defined(QUIET_ASSERT) || !defined(_INC_STDIO)
# define AssertAlways(e) do{ if(!(e)) { *(int*)0=0xA403; } }while(0)
# define AssertMsgAlways(e, msg) AssertAlways(e)
#else
# define AssertAlways(e) do{ if(!(e)){ \
printf(__FILE__"("stringify(__LINE__)"): Assert fail: "#e); \
fflush(stdout); DebugBreak; } }while(0)
# define AssertMsgAlways(e, msglit) do{ if(!(e)){ \
printf(__FILE__"("stringify(__LINE__)"): " msglit "\n"); \
fflush(stdout); DebugBreak; } }while(0)
#endif

#if RELEASE_MODE
# define Assert(expr)
# define AssertMsg(expr, msg)
#else
# define Assert(expr) AssertAlways(expr)
# define AssertMsg(expr, msg) AssertMsgAlways(expr, msg)
#endif

thread_local u32 ErrorNumber = 0;

////////////////////////////////
// intrinsics
////////////////////////////////

void mem_copy_non_overlapping(void *dst, const void *src, size_t len);
#define ZeroStruct(S) mem_zero(&(S), sizeof(S))
void mem_zero(void *data, size_t len);
int mem_compare(const void *str1, const void *str2, size_t count);

////////////////////////////////

typedef struct {
    const char *Data;
    size_t Len;
} view;
#define VIEW(cstr_lit) view_FromParts(cstr_lit, sizeof(cstr_lit) - 1)
#define VIEW_STATIC(cstr_lit) {(const char*)(cstr_lit), sizeof(cstr_lit) - 1}
#define VIEW_FMT "%.*s"
#define VIEW_ARG(v) (int)(v).Len, (v).Data

#ifndef VIEWPROC
# define VIEWPROC
#endif

int is_space(int _c);
VIEWPROC view view_FromParts(const char *Data, size_t Count);
VIEWPROC view view_FromCstr(const char *Cstr);
VIEWPROC bool view_Eq(view A, view B);
VIEWPROC view view_ChopByDelim(view *v, char Delim);
VIEWPROC view view_ChopByView(view *v, view Delim); // full view is the delim
VIEWPROC view view_ChopLeft(view *v, size_t n);
VIEWPROC view view_ChopRight(view *v, size_t n);
VIEWPROC view view_TrimLeft(view v);
VIEWPROC view view_TrimRight(view v);
VIEWPROC view view_Trim(view v);

#define PARSE_FAIL 0
#define PARSE_NO_DECIMALS 1
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
} memory_arena;

typedef struct {
    memory_arena *Arena;
    size_t StartMemOffset;
} scratch_arena;

#ifndef ARENAPROC
#define ARENAPROC
#endif

ARENAPROC void ArenaInit(memory_arena *Arena, size_t Size, void *Base);
#define ArenaGetRemaining(Arena, ...) \
SELECT_PROC_1DEFAULT(ArenaGetRemaining_align, ArenaGetRemaining_default, ##__VA_ARGS__)((Arena), ## __VA_ARGS__)
#define PushStruct(Arena, type, ...) (type*) \
SELECT_PROC_1DEFAULT(ArenaPushSize_align, ArenaPushSize_default, ##__VA_ARGS__)((Arena), sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type*) \
SELECT_PROC_1DEFAULT(ArenaPushSize_align, ArenaPushSize_default, ##__VA_ARGS__)((Arena), (Count)*sizeof(type), ## __VA_ARGS__)
ARENAPROC scratch_arena ArenaBeginScratch(memory_arena *Arena);
ARENAPROC void ArenaEndScratch(scratch_arena Scratch, bool ZeroMem);
#define ArenaClear(Arena, ZeroMem) if(ZeroMem) {mem_zero((Arena)->Base, (Arena)->Size);} (Arena)->Used = 0

ARENAPROC size_t ArenaGetAlignmentOffset(memory_arena *Arena, size_t Alignment);
ARENAPROC size_t ArenaGetRemaining_align(memory_arena *Arena, size_t Alignment);

#define ArenaGetRemaining_default(Arena) ArenaGetRemaining_align((Arena), 4)
#define ArenaPushSize_default(Arena, Size) ArenaPushSize_align((Arena), Size, 4)

////////////////////////////////

// IMPORTANT: Only check ErrorNumber when return is null (0)
#define ERROR_READ_UNKNOWN 1
#define ERROR_READ_FILE_NOT_FOUND 2
#define ERROR_READ_ACCESS_DENIED 3
#define ERROR_READ_NO_MEM 4
#define ERROR_READ_FILE_TOO_BIG 5 // READ_ENTIRE_FILE_MAX exceeded

#ifndef READ_ENTIRE_FILE_MAX
#define READ_ENTIRE_FILE_MAX 0xFFFFFFFF
#endif

// char *ReadEntireFile(const char *File, size_t *Size);
#define ReadEntireFile(A, B) (char*)0; (void)B; \
AssertMsgAlways(false, "To use 'ReadEntireFile' you must include both stdlib.h and stdio.h or either windows.h or fileapi.h (windows api)")

#if defined(_APISETFILE_) // windows fileapi.h included

#undef ReadEntireFile
#define ReadEntireFile win32_ReadEntireFile
char *win32_ReadEntireFile(const char *File, size_t *Size);

// windows fileapi.h included
#elif defined(_INC_STDIO) && (defined(_INC_MALLOC) || defined(_INC_STDLIB))

#undef ReadEntireFile
#define ReadEntireFile std_ReadEntireFile
// ERROR_READ_UNKNOWN - fopen, fseek, ftell failed
// fopen could fail due to many things
// TODO: Set ErrorNumber depending on them
// ERROR_READ_NO_MEM - malloc failed
// ERROR_READ_FILE_TOO_BIG - READ_ENTIRE_FILE_MAX exceeded
char *std_ReadEntireFile(const char *file, size_t *size);

#endif // stdio.h && (malloc.h || stdlib.h) included



#ifdef VICLIB_IMPLEMENTATION

int is_space(int _c)
{
    char c = (char)_c;
    return(c == ' ' || c == '\n' || c == '\t' || 
           c == '\v' || c == '\f' || c == '\r');
}

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
    v.Len = 0;
    for(; *Cstr != 0; Cstr++) v.Len++;
    return v;
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

VIEWPROC bool view_Eq(view A, view B)
{
    if(A.Len != B.Len) return false;
    else return mem_compare(A.Data, B.Data, A.Len) == 0;
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
    
    if(*v.Data > '9' || *v.Data < '0') return false;
    
    s64 Value = 0;
    int j = 0;
    for(; j < (int)v.Len; j++)
    {
        char c = v.Data[j];
        if(c != '_') {
            s64 Digit = (s64)_digit_val((int)c);
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
    
    int j = 0;
    for(; j < (int)v.Len; j++)
    {
        char c = v.Data[j];
        if(c == '.') {
            DecimalPart = true;
            if(j+1 < (int)v.Len && v.Data[j+1] == '.') {
                return PARSE_NO_DECIMALS;
            }
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
    printf("%lf\n", Val);
    if(Remaining) {
        Remaining->Data = v.Data + (size_t)j;
        Remaining->Len = v.Len - (size_t)j;
    }
    return PARSE_OK;
}

////////////////////////////////

void mem_copy_non_overlapping(void *dst, const void *src, size_t len)
{
    size_t i;
    /*
    * memcpy does not support overlapping buffers, so always do it
    * forwards. (Don't change this without adjusting memmove.)
    *
    * For speedy copying, optimize the common case where both pointers
    * and the length are word-aligned, and copy word-at-a-time instead
    * of byte-at-a-time. Otherwise, copy by bytes.
    *
    * The alignment logic below should be portable. We rely on
    * the compiler to be reasonably intelligent about optimizing
    * the divides and modulos out. Fortunately, it is.
    */
    if ((uintptr_t)dst % sizeof(long) == 0 &&
        (uintptr_t)src % sizeof(long) == 0 &&
        len % sizeof(long) == 0) {
        
        long *d = dst;
        const long *s = src;
        
        for(i = 0; i < len/sizeof(long); i++) {
            d[i] = s[i];
        }
    }
    else {
        char *d = dst;
        const char *s = src;
        
        for(i = 0; i < len; i++) {
            d[i] = s[i];
        }
    }
}

void mem_zero(void *data, size_t len)
{
    size_t i;
    
    if ((uintptr_t)data % sizeof(long) == 0 &&
        len % sizeof(long) == 0) {
        long *d = (long*)data;
        
        for(i = 0; i < len/sizeof(long); i++) {
            d[i] = 0;
        }
    }
    else {
        char *d = data;
        
        for(i = 0; i < len; i++) {
            d[i] = 0;
        }
    }
}

int mem_compare(const void *str1, const void *str2, size_t count)
{
    register const unsigned char *s1 = (const unsigned char*)str1;
    register const unsigned char *s2 = (const unsigned char*)str2;
    
    for(;count-- > 0;)
    {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

////////////////////////////////

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

ARENAPROC size_t ArenaGetRemaining_align(memory_arena *Arena, size_t Alignment)
{
    size_t Result = Arena->Size - (Arena->Used + ArenaGetAlignmentOffset(Arena, Alignment));
    return Result;
}

ARENAPROC void *ArenaPushSize_align(memory_arena *Arena, size_t RequestSize, size_t Alignment)
{
    size_t Size = RequestSize;
    size_t AlignOffset = ArenaGetAlignmentOffset(Arena, Alignment);
    Size += AlignOffset;
    
    AssertMsg((Arena->Used + Size) <= Arena->Size, "Assert Fail: Full arena size reached");
    void *Mem = Arena->Base + Arena->Used + AlignOffset;
    Arena->Used += Size;
    
    return Mem;
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

PUSH_IGNORE_UNINITIALIZED

char *win32_ReadEntireFile(const char *File, size_t *Size)
{
    AssertMsg(Size != 0, "Size parameter must be a valid pointer");
    char *Result;
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

// windows fileapi.h included
#elif defined(_INC_STDIO) && (defined(_INC_MALLOC) || defined(_INC_STDLIB))

PUSH_IGNORE_UNINITIALIZED

char *std_ReadEntireFile(const char *file, size_t *size)
{
    AssertMsg(size != 0, "Size parameter must not be a valid pointer");
    char *buffer = 0;
    FILE *f = fopen(file, "rb");
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
        buffer = malloc(sizeof(char) * m);
        ok = buffer != 0;
        if(!ok) ErrorNumber = ERROR_READ_NO_MEM;
    }
    
    if(ok) {
        ok = fseek(f, 0, SEEK_SET) >= 0;
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }
    
    if(ok) {
        size_t n = fread(buffer, 1, m, f);
        ok = n == (size_t)m;
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }
    
    if(ok) {
        ok = !ferror(f);
        if(!ok) ErrorNumber = ERROR_READ_UNKNOWN;
    }
    
    if(size) {
        *size = m;
    }
    
    if(f) fclose(f);
    if(!ok && buffer) free(buffer);
    
    return buffer;
}

RESTORE_WARNINGS

#endif // stdio.h && (malloc.h || stdlib.h) included

#endif // VICLIB_IMPLEMENTATION
#endif //VICLIB_H
