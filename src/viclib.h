/* date = December 29th 2024 10:12 pm
--Author: Víctor López Cortés
--Usage:
-Defines: To have any of these take effect, you must define them _before_ including this file
BASE_TYPES_IMPLEMENTATION if you want to have the implementation
READ_ENTIRE_FILE_MAX if you want to have a max file read size
QUIET_ASSERT if you want the assertions to add a breakpoint but not print
RELEASE_MODE to have some stuff work faster, right now, assertions get compiled out when this is defined

-Check ErrorNumber when errors occur.
WARNING I'm pretty sure doing it like this doesn't let you see what the error was when using multithreading when an error happens at the same time in different threads, but this should not happen since errors shouldn't occur 99% of the time anyway. I will however change this if I see it's not great.
Right now, only std_ReadEntireFile sets this.

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

#if defined(__gnu_linux__)
# define OS_LINUX 1
#endif

#if defined(__APPLE__) && defined(__MACH__)
# define OS_MAC 1
#endif

#if !defined(__COLUMN__)
# define __COLUMN__ 0
#endif

#if defined(__func__)
# define __PROC__ __func__
#elif defined(__FUNCTION__)
# define __PROC__ __FUNCTION__
#else
# define __PROC__ ""
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
#elif SDL_HAS_BUILTIN(__builtin_debugtrap)
# define DebugBreak __builtin_debugtrap()
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
# define DebugBreak __asm__ __volatile__ ( "int $3\n\t" )
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__riscv)
# define DebugBreak __asm__ __volatile__ ( "ebreak\n\t" )
#elif ( defined(SDL_PLATFORM_APPLE) && (defined(__arm64__) || defined(__aarch64__)) )  /* this might work on other ARM targets, but this is a known quantity... */
# define DebugBreak __asm__ __volatile__ ( "brk #22\n\t" )
#elif defined(SDL_PLATFORM_APPLE) && defined(__arm__)
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
# define DebugBreak
#endif

// only works with static arrays!
#define ArrayLen(arr) sizeof(arr)/sizeof(arr[0])

#define stringify_(a) #a
#define stringify(a) stringify_(a)

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
printf(__FILE__"("stringify(__LINE__"): Assert fail: "#e)); \
fflush(stdout); DebugBreak; } }while(0)
# define AssertMsgAlways(e, msglit) do{ if(!(e)){ \
printf(__FILE__"("stringify(__LINE__"): Assert fail: " msglit)); \
fflush(stdout); DebugBreak; } }while(0)
#endif

#if RELEASE_MODE
# define Assert(expr)
# define AssertMsg(expr, msg)
#else
# define Assert(expr) AssertAlways(expr)
# define AssertMsg(expr, msg) AssertMsgAlways(expr, msg)
#endif

u32 ErrorNumber = 0;

////////////////////////////////
// intrinsics
////////////////////////////////

void mem_copy_non_overlapping(void *dst, const void *src, size_t len);
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

int isspace(int _c);
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

VIEWPROC bool view_ParseS64(view v, s64 *Result, int *IdxAfterNum);

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

#if defined(_INC_STDIO) && defined(_INC_MALLOC)

// IMPORTANT: Only check ErrorNumber when return is null (0)
#define ERROR_READ_UNKNOWN 1 // fopen, fseek, ftell failed
#define ERROR_READ_NO_MEM 2 // malloc failed
#define ERROR_READ_FILE_TOO_BIG 3 // READ_ENTIRE_FILE_MAX exceeded
char *std_ReadEntireFile(const char *file, size_t *size);

#endif // stdio.h && malloc.h included

#ifdef VICLIB_IMPLEMENTATION

int isspace(int _c)
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
    for(;i < v.Len && isspace(v.Data[i]);) i += 1;
    
    return view_FromParts((const char*)(v.Data + i), v.Len - i);
}
VIEWPROC view view_TrimRight(view v)
{
    size_t i = 0;
    for(;i < v.Len && isspace(v.Data[v.Len - 1 - i]);) i += 1;
    
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

// NOTE: If I ever require using octal like in C,
// '0' is the prefix for octal, surprisingly, so '080' is an invalid number
// try to compile: int n = 080;
// I checked out how Odin did octal and they do "0o" prefix, 
// I also don't know what the "0z" prefix of base 12 is for but I'll just leave it there
VIEWPROC bool view_ParseS64(view v, s64 *Result, int *IdxAfterNum)
{
    AssertMsg(Result != 0, "Result parameter must be a valid pointer");
    if(v.Len == 0) return false;
    size_t IniLen = v.Len;
    
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
    
    s64 Value = 0;
    int i = 0;
    for(int j = 0; j < v.Len; j++)
    {
        char c = v.Data[j];
        if(c == '_') {
            i++;
            continue;
        }
        
        s64 Digit = (s64)_digit_val((int)c);
        if(Digit >= Base) {
            // invalid digit
            break;
        }
        Value *= Base;
        Value += Digit;
        i++;
    }
    
    if(Neg) Value = -Value;
    *Result = Value;
    if(IdxAfterNum) *IdxAfterNum = (int)(IniLen - v.Len + (size_t)i);
    
    return true;
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

#if defined(_INC_STDIO) && defined(_INC_MALLOC)

char *std_ReadEntireFile(const char *file, size_t *size)
{
    char *buffer = 0;
    FILE *f = fopen(file, "rb");
    if(f == 0) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        goto std_readentirefileerror;
    }
    
    if(fseek(f, 0, SEEK_END) < 0) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        goto std_readentirefileerror;
    }
    
    long m = ftell(f);
#ifdef READ_ENTIRE_FILE_MAX
    if(m > READ_ENTIRE_FILE_MAX) {
        ErrorNumber = ERROR_READ_FILE_TOO_BIG;
        goto std_readentirefileerror;
    }
#endif
    if(m < 0) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        goto std_readentirefileerror;
    }
    
    buffer = malloc(sizeof(char) * m);
    if(buffer == 0) {
        ErrorNumber = ERROR_READ_NO_MEM;
        goto std_readentirefileerror;
    }
    
    if(fseek(f, 0, SEEK_SET) < 0) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        goto std_readentirefileerror;
    }
    
    size_t n = fread(buffer, 1, m, f);
    if(n != (size_t)m) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        goto std_readentirefileerror;
    }
    
    if(ferror(f)) {
        ErrorNumber = ERROR_READ_UNKNOWN;
        goto std_readentirefileerror;
    }
    
    if(size) {
        *size = n;
    }
    
    fclose(f);
    
    return buffer;
    
    std_readentirefileerror:
    if(f) {
        fclose(f);
    }
    
    if(buffer) {
        free(buffer);
    }
    
    return NULL;
}

#endif // stdio.h && malloc.h included

#endif // VICLIB_IMPLEMENTATION
#endif //VICLIB_H
