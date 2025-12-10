// [vl_build.h](https://github.com/victor-Lopez25/viclib) © 2024 by [Víctor López Cortés](https://github.com/victor-Lopez25) is licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)
// version: 1.3.3
#ifndef VL_BUILD_H
#define VL_BUILD_H

// Huge TODO: Try to do this without stdlib.h?
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#ifndef VICLIB_PATH
# define VICLIB_PATH "viclib.h"
#endif

#if defined(VL_BUILD_IMPLEMENTATION) && !defined(VICLIB_IMPLEMENTATION)
# define VICLIB_IMPLEMENTATION
#endif

#if defined(_WIN32)
# if !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
# endif
# define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
typedef HANDLE vl_proc;
# define VL_INVALID_PROC INVALID_HANDLE_VALUE
typedef HANDLE vl_fd;
# define VL_INVALID_FD INVALID_HANDLE_VALUE
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
typedef int vl_proc;
# define VL_INVALID_PROC (-1)
typedef int vl_fd;
# define VL_INVALID_FD (-1)
#endif

#define VL_INC_STDIO_H
#define VL_INC_STDLIB_H
#define VL_INC_STRING_H
#include VICLIB_PATH

#define VL_REALLOC realloc
#define VL_FREE free

VLIBPROC vl_fd fd_OpenForRead(const char *path);
VLIBPROC vl_fd fd_OpenForWrite(const char *path);
VLIBPROC void fd_Close(vl_fd fd);

#if defined(__GNUC__) || defined(__clang__)
//   https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
#    ifdef __MINGW_PRINTF_FORMAT
#        define VL_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
#    else
#        define VL_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
#    endif // __MINGW_PRINTF_FORMAT
#else
// MSVC can't do this iirc
#    define VL_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

#if COMPILER_CL
#define VL_TODO(msg) PRAGMA(message(LOC_MSVC_STR ": TODO: " msg))
#else
#define VL_TODO(msg) PRAGMA(message("TODO: " msg))
#endif

#ifndef PRINT_TIME_Fmt
# define PRINT_TIME_Fmt "%llumins %02usecs %03u.%03u%03ums"
#endif

#ifndef PRINT_TIME_Arg
# define PRINT_TIME_Arg(time) (time/60000000000), (uint32_t)((time/1000000000)%60), \
    (uint32_t)((time/1000000)%1000), (uint32_t)((time/1000)%1000), (uint32_t)(time%1000)
#endif

#define LogTimeBetween(stmt, msg) \
    do { \
        u64 glue(timer_, __LINE__) = VL_GetNanos(); \
        stmt;                                             \
        glue(timer_, __LINE__) = VL_GetNanos() - glue(timer_, __LINE__); \
        VL_Log(VL_INFO, "%sTime taken: "PRINT_TIME_Fmt, msg, PRINT_TIME_Arg(glue(timer_, __LINE__))); \
    } while(0)

typedef enum {
    VL_ECHO,
    VL_INFO,
    VL_WARNING,
    VL_ERROR,
    VL_QUIET = 8, // in case you want to put something before
} vl_log_level;

extern vl_log_level VL_MinimalLogLevel;
VLIBPROC void VL_Log(vl_log_level lvl, const char *fmt, ...) VL_PRINTF_FORMAT(2, 3);

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} vl_file_paths;

struct VL_CopyDirectoryRecursively_opts {
    const char *src;
    const char *dst;
    const char *ext;
};

VLIBPROC bool MkdirIfNotExist(const char *path);
VLIBPROC bool VL_CopyFile(const char *src, const char *dst);
#define VL_CopyDirectoryRecursively(src_path, ...) \
    VL_CopyDirectoryRecursively_Opt((struct VL_CopyDirectoryRecursively_opts){.src = (src_path), __VA_ARGS__})
VLIBPROC bool VL_CopyDirectoryRecursively_Impl(const char *src_path, const char *dst_path, const char *ext);
VLIBPROC bool VL_CopyDirectoryRecursively_Opt(struct VL_CopyDirectoryRecursively_opts opt);
VLIBPROC bool VL_ReadDirectoryFilesRecursively(const char *parent, vl_file_paths *children);
VLIBPROC bool VL_ReadEntireDir(const char *parent, vl_file_paths *children);
VLIBPROC bool VL_WriteEntireFile(const char *path, const void *data, size_t size);
VLIBPROC bool VL_DeleteFile(const char *path);

// Initial capacity of a dynamic array
#ifndef VL_DA_INIT_CAP
# define VL_DA_INIT_CAP 256
#endif

#ifdef __cplusplus
#define VL_DECLTYPE_CAST(T) (decltype(T))
#else
#define VL_DECLTYPE_CAST(T)
#endif

#define da_Reserve(da, ExpectedCapacity)                                                 \
    do {                                                                                 \
        if((ExpectedCapacity) > (da)->capacity) {                                        \
            if((da)->capacity == 0) {                                                    \
                (da)->capacity = VL_DA_INIT_CAP;                                         \
            }                                                                            \
            while((ExpectedCapacity) > (da)->capacity) {                                 \
                (da)->capacity *= 2;                                                     \
            }                                                                            \
            (da)->items = VL_DECLTYPE_CAST((da)->items)VL_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            Assert((da)->items != NULL && "Buy more RAM lol");                           \
        }                                                                                \
    } while(0)

#define da_AppendMany(da, NewItems, NewItemCount)                                       \
    do {                                                                                \
        da_Reserve((da), (da)->count + (NewItemCount));                                 \
        memcpy((da)->items + (da)->count, (NewItems), (NewItemCount)*sizeof(*(da)->items)); \
        (da)->count += (NewItemCount);                                                  \
    } while(0)

#define da_Append(da, item)                  \
    do {                                     \
        da_Reserve((da), (da)->count + 1);   \
        (da)->items[(da)->count++] = (item); \
    } while(0)

#define da_Free(da) VL_FREE((da).items)
#define da_RemoveUnordered(da, i)                    \
    do {                                             \
        size_t j = (i);                              \
        Assert(j < (da)->count);                     \
        (da)->items[j] = (da)->items[--(da)->count]; \
    } while(0)

#define da_Foreach(Type, it, da) for(Type *it = (da)->items; it < (da)->items + (da)->count; ++it)

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} string_builder;

VLIBPROC bool sb_ReadEntireFile(const char *path, string_builder *sb);
// Does not append null terminator to sb
VLIBPROC int sb_Appendf(string_builder *sb, const char *fmt, ...) VL_PRINTF_FORMAT(2, 3);
VLIBPROC bool sb_PadAlign(string_builder *sb, size_t size);

#define sb_AppendBuf(sb, buf, size) da_AppendMany(sb, buf, size)
// does not include null character
#define sb_AppendCstr(sb, cstr)  \
    do {                         \
        const char *s = (cstr);  \
        size_t n = strlen(s);    \
        da_AppendMany(sb, s, n); \
    } while (0)

#define sb_AppendNull(sb) da_Append(sb, 0)
#define sb_Free(sb) VL_FREE((sb).items)

typedef struct {
    vl_proc *items;
    size_t count;
    size_t capacity;
} vl_procs;

// Wait until the process has finished
VLIBPROC bool VL_ProcWait(vl_proc proc);

// Pretty hard to understand, so it's marked as private
VLIBPROC int VL__ProcWaitAsync(vl_proc proc, int ms);

// Wait until all the processes have finished
VLIBPROC bool VL_ProcsWait(vl_procs procs);

// Wait until all the processes have finished and empty the procs array.
VLIBPROC bool VL_ProcsFlush(vl_procs *procs);

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
#if OS_WINDOWS
    bool msvc_linkflags; // need some way to know if you specified /link at some point
#endif
} vl_cmd;

typedef struct {
    vl_cmd *cmd;
    // Run the command asynchronously appending its VL_Proc to the provided VL_Procs array
    vl_procs *async;
    // Maximum processes in the .async list. 
    size_t maxProcs;
    // redirect to file or 'nul' for /dev/null or nul on windows
    const char *stdinPath;
    const char *stdoutPath;
    const char *stderrPath;
} vl_cmd_opts;

// Render a string representation of a command into a string builder. Keep in mind the the
// string builder is not NULL-terminated by default. Use sb_AppendNull if you plan to
// use it as a C string.
VLIBPROC void VL_CmdRender(vl_cmd cmd, string_builder *render);

VLIBPROC bool CmdRun_Opt(vl_cmd_opts opt);
#define CmdRun(Cmd, ...) CmdRun_Opt((vl_cmd_opts){.cmd = (Cmd), __VA_ARGS__})

#define cmd_Append(cmd, ...) \
    da_AppendMany(cmd, \
                  ((const char*[]){__VA_ARGS__}), \
                  (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))

#define cmd_Extend(cmd, other_cmd) \
    da_AppendMany(cmd, (other_cmd)->items, (other_cmd)->count)

#define cmd_Free(cmd) VL_FREE(cmd.items)

VLIBPROC int VL_GetCountProcs(void);

VLIBPROC char *temp_sprintf(const char *fmt, ...) VL_PRINTF_FORMAT(1, 2);

#ifndef VL_PUSHD_BUF_MAX
# define VL_PUSHD_BUF_MAX 32
#endif

struct vl__pushd_buf_type {
    const char *items[VL_PUSHD_BUF_MAX];
    int count;
};
extern struct vl__pushd_buf_type VL__pushDirectoryBuffer;

// Given any path returns the last part of that path.
// "/path/to/a/file.c" -> "file.c"; "/path/to/a/directory" -> "directory"
VLIBPROC const char *VL_PathName(const char *path);
VLIBPROC bool VL_Rename(const char *old, const char *New);
VLIBPROC int VL_FileExists(const char *path);
VLIBPROC const char *VL_temp_GetCurrentDir(void);
VLIBPROC bool VL_SetCurrentDir(const char *path);
VLIBPROC bool VL_Pushd(const char *path);
VLIBPROC bool VL_Popd(void);
// Returns you the directory part of the path allocated on the temporary storage.
VLIBPROC char *VL_temp_DirName(const char *path);
VLIBPROC char *VL_temp_FileName(const char *path);
VLIBPROC char *VL_temp_FileExt(const char *path);
VLIBPROC char *VL_temp_RunningExecutablePath(void);

VLIBPROC bool VL_GetLastWriteTime(const char *file, u64 *writeTime);

// Only checks the filetime of all the input files vs output file
#define VL_NeedsRebuild(out, in, ...) VL_NeedsRebuild_Impl(out, ((const char*[]){in, __VA_ARGS__}), sizeof((const char*[]){in, __VA_ARGS__})/sizeof(const char*))
VLIBPROC int VL_NeedsRebuild_Impl(const char *output_path, const char **input_paths, size_t input_paths_count);

typedef struct vl_filetime_node vl_filetime_node;
struct vl_filetime_node {
    view file;
    u64 time;
    //bool exploredAllDependencies;
    vl_filetime_node *next;
};

#ifndef VL_BUILD_FILETIME_TABLE_SIZE
#define VL_BUILD_FILETIME_TABLE_SIZE 1024
#endif // VL_BUILD_FILETIME_TABLE_SIZE

typedef struct {
    vl_filetime_node *items;
    size_t count;
    size_t capacity;
} vl_filetime_nodelist;

typedef struct {
    vl_filetime_nodelist nodes;
    uint32_t countTimes;
} vl_filetime_table;

typedef struct {
    vl_filetime_table table;
    vl_filetime_node *freelist;
    vl_file_paths *includePaths;
    memory_arena *Arena;
} vl_needrebuild_context;

extern vl_needrebuild_context VL_needsRebuildContext;

#ifndef VL_BUILD_FILENAME_HASH
#define VL_BUILD_FILENAME_HASH(v, hash) do {\
    /* djb2 */ \
    hash = 5381; \
    for(int hashIdx = 0; hashIdx < (int)v.count; hashIdx++) \
        hash = ((hash << 5) + hash) + v.items[hashIdx]; /* hash * 33 + c */ \
    } while(0)
#endif // VL_BUILD_FILENAME_HASH

// Checks the filetime of all input files and all files #included by them
#define VL_Needs_C_Rebuild(out, in, ...) VL_Needs_C_Rebuild_Impl(out, ((const char*[]){in, __VA_ARGS__}), sizeof((const char*[]){in, __VA_ARGS__})/sizeof(const char*))
VLIBPROC int VL_Needs_C_Rebuild_Impl(const char *output_path, const char **input_paths, size_t input_paths_count);

typedef enum {
    // 0 is default so if none is chosen, use the current compiler
#if COMPILER_GCC
    CCompiler_GCC = 0,
    CCompiler_Clang,
    CCompiler_MSVC,
#elif COMPILER_CLANG
    CCompiler_Clang = 0,
    CCompiler_GCC,
    CCompiler_MSVC,
#elif COMPILER_CL
    CCompiler_MSVC = 0,
    CCompiler_Clang,
    CCompiler_GCC,
#endif
} vl_c_compiler;

struct compiler_info_opts {
    vl_cmd *cmd;
    vl_c_compiler cc;
};

#if OS_WINDOWS
# define VL_EXE_EXTENSION ".exe"
#else
# define VL_EXE_EXTENSION
#endif

VLIBPROC void VL_cc_Opt(struct compiler_info_opts opt);
#define VL_cc(Cmd, ...) VL_cc_Opt((struct compiler_info_opts){.cmd = (Cmd), __VA_ARGS__})

VLIBPROC void VL_ccWarnings_Opt(struct compiler_info_opts opt);
#define VL_ccWarnings(Cmd, ...) VL_ccWarnings_Opt((struct compiler_info_opts){.cmd = (Cmd), __VA_ARGS__})

VLIBPROC void VL_ccWarningsAsErrors_Opt(struct compiler_info_opts opt);
#define VL_ccWarningsAsErrors(Cmd, ...) VL_ccWarningsAsErrors_Opt((struct compiler_info_opts){.cmd = (Cmd), __VA_ARGS__})

VLIBPROC void VL_ccOutput_Opt(struct compiler_info_opts opt, const char *output);
#define VL_ccOutput(Cmd, out, ...) VL_ccOutput_Opt((struct compiler_info_opts){.cmd = (Cmd), __VA_ARGS__}, out)

VLIBPROC void VL_ccDebug_Opt(struct compiler_info_opts opt);
#define VL_ccDebug(Cmd, ...) VL_ccDebug_Opt((struct compiler_info_opts){.cmd = (Cmd), __VA_ARGS__})

VLIBPROC void VL_ccLibs_Opt(struct compiler_info_opts opt, const char **libs, size_t libcount);
#define VL_ccLibs(Cmd, l1, ...) VL_ccLibs_Opt((struct compiler_info_opts){.cmd = (Cmd)}, \
    ((const char*[]){l1, __VA_ARGS__}), sizeof((const char*[]){l1, __VA_ARGS__})/sizeof(const char*))
#define VL_ccLibsC(Cmd, CC, l1, ...) VL_ccLibs_Opt((struct compiler_info_opts){.cmd = (Cmd), .cc = (CC)}, \
    ((const char*[]){l1, __VA_ARGS__}), sizeof((const char*[]){l1, __VA_ARGS__})/sizeof(const char*))

VLIBPROC void VL_ccLibpath_Opt(struct compiler_info_opts opt, const char *libpath);
#define VL_ccLibpath(Cmd, libpath, ...) VL_ccLibpath_Opt((struct compiler_info_opts){.cmd = (Cmd), __VA_ARGS__}, libpath)

#if defined(_WIN32)
# if defined(_MSC_VER)
#  define VL_DEFAULT_REBUILD_URSELF(bin_path, src_path) "cl.exe", "-nologo", "-FC", "-GR-", "-EHa", temp_sprintf("/Fe:%s", bin_path), src_path, "-W4", "-D_CRT_SECURE_NO_WARNINGS"
#  define VL_CC_DEBUG_INFO "-Zi"
# elif defined(__GNUC__)
#  define VL_DEFAULT_REBUILD_URSELF(bin_path, src_path) "gcc", "-o", bin_path, src_path, "-Wall", "-Wextra"
#  define VL_CC_DEBUG_INFO "-g"
# elif defined(__clang__)
#  define VL_DEFAULT_REBUILD_URSELF(bin_path, src_path) "clang", "-o", bin_path, src_path, "-Wall", "-Wextra"
#  define VL_CC_DEBUG_INFO "-g"
# endif
#else
# define VL_DEFAULT_REBUILD_URSELF(bin_path, src_path) "cc", "-o", bin_path, src_path, "-Wall", "-Wextra"
# define VL_CC_DEBUG_INFO "-g"
#endif

#ifndef VL_REBUILD_URSELF
// In case you just want to add something in, like a "-D" which works on every compiler the same
# define VL_REBUILD_URSELF(bin_path, src_path) VL_DEFAULT_REBUILD_URSELF(bin_path, src_path)
#endif

// stolen from nob.h, made it better (imo)
VLIBPROC void VL__GoRebuildUrself(int argc, char **argv, const char **src_paths, size_t path_count);
#define VL_GO_REBUILD_URSELF(argc, argv, ...) VL__GoRebuildUrself(argc, argv, ((const char*[]){__FILE__, __VA_ARGS__}), sizeof((const char*[]){__FILE__, __VA_ARGS__})/sizeof(const char*))

#if OS_WINDOWS
VLIBPROC char *Win32_ErrorMessage(DWORD err);
#endif

#ifdef VL_BUILD_IMPLEMENTATION

static vl_proc VL__CmdStartProcess(vl_cmd cmd, vl_fd *fdin, vl_fd *fdout, vl_fd *fderr);

vl_log_level VL_MinimalLogLevel = VL_ECHO;

#if !OS_WINDOWS
#include <dirent.h>
#endif

#if OS_WINDOWS

struct dirent {
    char d_name[MAX_PATH+1];
};

typedef struct DIR DIR;

static DIR *opendir(const char *dirpath);
static struct dirent *readdir(DIR *dirp);
static int closedir(DIR *dirp);

// Base on https://stackoverflow.com/a/75644008
// > .NET Core uses 4096 * sizeof(WCHAR) buffer on stack for FormatMessageW call. And...thats it.
// >
// > https://github.com/dotnet/runtime/blob/3b63eb1346f1ddbc921374a5108d025662fb5ffd/src/coreclr/utilcode/posterror.cpp#L264-L265
#ifndef VL_WIN32_ERR_MSG_SIZE
#define VL_WIN32_ERR_MSG_SIZE (4 * 1024)
#endif // VL_WIN32_ERR_MSG_SIZE

VLIBPROC char *Win32_ErrorMessage(DWORD err)
{
    static char win32ErrMsg[VL_WIN32_ERR_MSG_SIZE] = {0};
    DWORD errMsgSize = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, LANG_USER_DEFAULT, win32ErrMsg,
                                      VL_WIN32_ERR_MSG_SIZE, NULL);

    if(errMsgSize == 0) {
        if(GetLastError() != ERROR_MR_MID_NOT_FOUND) {
            if(sprintf(win32ErrMsg, "Could not get error message for 0x%lX", err) > 0) {
                return (char*)&win32ErrMsg;
            } else {
                return NULL;
            }
        } else {
            if(sprintf(win32ErrMsg, "Invalid Windows Error code (0x%lX)", err) > 0) {
                return (char*)&win32ErrMsg;
            } else {
                return NULL;
            }
        }
    }

    while(errMsgSize > 1 && isspace(win32ErrMsg[errMsgSize - 1])) {
        win32ErrMsg[--errMsgSize] = '\0';
    }

    return win32ErrMsg;
}
#endif

VLIBPROC vl_fd fd_OpenForRead(const char *path)
{
#ifndef _WIN32
    vl_fd result = open(path, O_RDONLY);
    if(result < 0) {
        VL_Log(VL_ERROR, "Could not open file %s: %s", path, strerror(errno));
        return VL_INVALID_FD;
    }
    return result;
#else
    // https://docs.microsoft.com/en-us/windows/win32/fileio/opening-a-file-for-reading-or-writing
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    vl_fd result = CreateFile(
                    path,
                    GENERIC_READ,
                    0,
                    &saAttr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_READONLY,
                    NULL);

    if(result == INVALID_HANDLE_VALUE) {
        VL_Log(VL_ERROR, "Could not open file %s: %s", path, Win32_ErrorMessage(GetLastError()));
        return VL_INVALID_FD;
    }

    return result;
#endif // _WIN32
}

VLIBPROC vl_fd fd_OpenForWrite(const char *path)
{
    // TODO: > /dev/null & > nul
#ifndef _WIN32
    vl_fd result = open(path,
                     O_WRONLY | O_CREAT | O_TRUNC,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(result < 0) {
        VL_Log(VL_ERROR, "Could not open file %s: %s", path, strerror(errno));
        return VL_INVALID_FD;
    }
    return result;
#else
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    vl_fd result = CreateFile(
                    path,                            // name of the write
                    GENERIC_WRITE,                   // open for writing
                    0,                               // do not share
                    &saAttr,                         // default security
                    CREATE_ALWAYS,                   // create always
                    FILE_ATTRIBUTE_NORMAL,           // normal file
                    NULL                             // no attr. template
                );

    if(result == INVALID_HANDLE_VALUE) {
        VL_Log(VL_ERROR, "Could not open file %s: %s", path, Win32_ErrorMessage(GetLastError()));
        return VL_INVALID_FD;
    }

    return result;
#endif // _WIN32
}

VLIBPROC void fd_Close(vl_fd fd)
{
#ifdef _WIN32
    CloseHandle(fd);
#else
    close(fd);
#endif // _WIN32
}

VLIBPROC void VL_Log(vl_log_level lvl, const char *fmt, ...)
{
    if(lvl < VL_MinimalLogLevel) return;

    switch(lvl) {
        case VL_ECHO:
        case VL_INFO: {
            fprintf(stderr, "[INFO] ");
        } break;
        case VL_WARNING: {
            fprintf(stderr, "[WARNING] ");
        } break;
        case VL_ERROR: {
            fprintf(stderr, "[ERROR] ");
        } break;
        case VL_QUIET: return;
        default: Assert(!"Unreachable");
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

VLIBPROC bool MkdirIfNotExist(const char *path)
{
#if OS_WINDOWS
    int result = _mkdir(path);
#else
    int result = mkdir(path, 0755);
#endif
    if(result < 0) {
        if(errno == EEXIST) {
            VL_Log(VL_ECHO, "directory `%s` already exists", path);
            return true;
        }
        VL_Log(VL_ERROR, "could not create directory `%s`: %s", path, strerror(errno));
        return false;
    }

    VL_Log(VL_ECHO, "created directory `%s`", path);
    return true;
}

VLIBPROC bool VL_CopyFile(const char *src, const char *dst)
{
    VL_Log(VL_ECHO, "copying %s -> %s", src, dst);
#if OS_WINDOWS
    if(!CopyFile(src, dst, false)) {
        VL_Log(VL_ERROR, "Could not copy file: %s", Win32_ErrorMessage(GetLastError()));
        return false;
    }
    return true;
#else
    int src_fd = -1;
    int dst_fd = -1;
    size_t bufSize = 32*1024;
    size_t tempMark = temp_save();
    char *buf = (char*)temp_alloc(bufSize, .Alignment = 1);

    Assert(buf != NULL && "Buy more RAM lol!!");
    bool result = true;

    src_fd = open(src, O_RDONLY);
    if(src_fd < 0) {
        VL_Log(VL_ERROR, "Could not open file %s: %s", src, strerror(errno));
        VL_ReturnDefer(false);
    }

    struct stat src_stat;
    if(fstat(src_fd, &src_stat) < 0) {
        VL_Log(VL_ERROR, "Could not get mode of file %s: %s", src, strerror(errno));
        VL_ReturnDefer(false);
    }

    dst_fd = open(dst, O_CREAT | O_TRUNC | O_WRONLY, src_stat.st_mode);
    if(dst_fd < 0) {
        VL_Log(VL_ERROR, "Could not create file %s: %s", dst, strerror(errno));
        VL_ReturnDefer(false);
    }

    for(;;) {
        ssize_t n = read(src_fd, buf, bufSize);
        if(n == 0) break;
        if(n < 0) {
            VL_Log(VL_ERROR, "Could not read from file %s: %s", src, strerror(errno));
            VL_ReturnDefer(false);
        }
        char *buf2 = buf;
        while (n > 0) {
            ssize_t m = write(dst_fd, buf2, n);
            if(m < 0) {
                VL_Log(VL_ERROR, "Could not write to file %s: %s", dst, strerror(errno));
                VL_ReturnDefer(false);
            }
            n    -= m;
            buf2 += m;
        }
    }

defer:
    temp_rewind(tempMark);
    close(src_fd);
    close(dst_fd);
    return result;
#endif
}

VLIBPROC bool VL_CopyDirectoryRecursively_Impl(const char *src, const char *dst, const char *ext)
{
    bool result = true;
    vl_file_paths children = {0};
    string_builder srcSb = {0};
    string_builder dstSb = {0};
    size_t tempCheckpoint = temp_save();

    file_type type = VL_GetFileType(src);
    if(type < 0) return false;

    switch(type) {
        case VL_FILE_DIRECTORY: {
            if(!MkdirIfNotExist(dst)) VL_ReturnDefer(false);
            if(!VL_ReadEntireDir(src, &children)) VL_ReturnDefer(false);

            for(size_t i = 0; i < children.count; i++) {
                if(!strcmp(children.items[i], ".")) continue;
                if(!strcmp(children.items[i], "..")) continue;

                srcSb.count = 0;
                sb_AppendCstr(&srcSb, src);
                sb_AppendCstr(&srcSb, "/");
                sb_AppendCstr(&srcSb, children.items[i]);
                sb_AppendNull(&srcSb);

                dstSb.count = 0;
                sb_AppendCstr(&dstSb, dst);
                sb_AppendCstr(&dstSb, "/");
                sb_AppendCstr(&dstSb, children.items[i]);
                sb_AppendNull(&dstSb);

                if(!VL_CopyDirectoryRecursively_Impl(srcSb.items, dstSb.items, ext)) {
                    VL_ReturnDefer(false);
                }
            }
        } break;

        case VL_FILE_REGULAR: {
            if(view_EndsWith(view_FromCstr(src), view_FromCstr(ext)) && !VL_CopyFile(src, dst)) {
                VL_ReturnDefer(false);
            }
        } break;

        case VL_FILE_SYMLINK: {
            VL_Log(VL_WARNING, "TODO: Copying symlinks is not supported yet");
        } break;

        case VL_FILE_OTHER: {
            VL_Log(VL_ERROR, "Unsupported type of file %s", src);
            VL_ReturnDefer(false);
        } break;

        default: Assert(!"Unreachable");
    }

defer:
    temp_rewind(tempCheckpoint);
    da_Free(srcSb);
    da_Free(dstSb);
    da_Free(children);
    return result;
}

VLIBPROC bool VL_CopyDirectoryRecursively_Opt(struct VL_CopyDirectoryRecursively_opts opt)
{
    AssertMsg(opt.src != 0, "Invalid parameter: src directory is null");
    if(opt.dst == 0) opt.dst = ".";
    if(opt.ext == 0) opt.ext = "";

    VL_Log(VL_ECHO, "copying %s/*%s -> %s", opt.src, opt.ext, opt.dst);

    vl_log_level prevLogLevel = VL_MinimalLogLevel;
    VL_MinimalLogLevel = VL_INFO;
    bool ok = VL_CopyDirectoryRecursively_Impl(opt.src, opt.dst, opt.ext);
    VL_MinimalLogLevel = prevLogLevel;

    return ok;
}

VLIBPROC bool VL_ReadDirectoryFilesRecursively(const char *parent, vl_file_paths *children)
{
    bool result = true;
    
    vl_file_paths children_tmp = {0};
    string_builder src_sb = {0};

    file_type type = VL_GetFileType(parent);
    if(type < 0) return false;

    switch(type) {
        case VL_FILE_DIRECTORY: {
            if(!VL_ReadEntireDir(parent, &children_tmp)) VL_ReturnDefer(false);

            for(size_t i = 0; i < children_tmp.count; ++i) {
                if(strcmp(children_tmp.items[i], ".") == 0) continue;
                if(strcmp(children_tmp.items[i], "..") == 0) continue;

                src_sb.count = 0;
                sb_AppendCstr(&src_sb, parent);
                sb_AppendCstr(&src_sb, "/");
                sb_AppendCstr(&src_sb, children_tmp.items[i]);
                sb_AppendNull(&src_sb);

                if(!VL_ReadDirectoryFilesRecursively(src_sb.items, children)) {
                    VL_ReturnDefer(false);
                }
            }
        } break;

        case VL_FILE_REGULAR:
        case VL_FILE_SYMLINK: {
            da_Append(children, temp_strdup(parent));
        } break;

        case VL_FILE_OTHER: {
            VL_Log(VL_ERROR, "Unsupported type of file %s", parent);
            VL_ReturnDefer(false);
        } break;

        default: Assert(!"unreachable");
    }

defer:
    da_Free(children_tmp);
    da_Free(src_sb);
    return result;
}

VLIBPROC bool VL_ReadEntireDir(const char *parent, vl_file_paths *children)
{
    bool result = true;
    DIR *dir = NULL;
    struct dirent *ent = NULL;

    dir = opendir(parent);
    if (dir == NULL) {
        #ifdef _WIN32
        VL_Log(VL_ERROR, "Could not open directory %s: %s", parent, Win32_ErrorMessage(GetLastError()));
        #else
        VL_Log(VL_ERROR, "Could not open directory %s: %s", parent, strerror(errno));
        #endif // _WIN32
        VL_ReturnDefer(false);
    }

    errno = 0;
    ent = readdir(dir);
    while (ent != NULL) {
        da_Append(children, temp_strdup(ent->d_name));
        ent = readdir(dir);
    }

    if (errno != 0) {
        #ifdef _WIN32
        VL_Log(VL_ERROR, "Could not read directory %s: %s", parent, Win32_ErrorMessage(GetLastError()));
        #else
        VL_Log(VL_ERROR, "Could not read directory %s: %s", parent, strerror(errno));
        #endif // _WIN32
        VL_ReturnDefer(false);
    }

defer:
    if (dir) closedir(dir);
    return result;
}

VLIBPROC bool VL_WriteEntireFile(const char *path, const void *data, size_t size)
{
    bool result = true;

    const char *buf = NULL;
    FILE *f = fopen(path, "wb");
    if(f == NULL) {
        VL_Log(VL_ERROR, "Could not open file %s for writing: %s\n", path, strerror(errno));
        VL_ReturnDefer(false);
    }

    //           len
    //           v
    // aaaaaaaaaa
    //     ^
    //     data

    buf = (const char*)data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, f);
        if(ferror(f)) {
            VL_Log(VL_ERROR, "Could not write into file %s: %s\n", path, strerror(errno));
            VL_ReturnDefer(false);
        }
        size -= n;
        buf  += n;
    }

defer:
    if(f) fclose(f);
    return result;
}

VLIBPROC bool VL_DeleteFile(const char *path)
{
    VL_Log(VL_ECHO, "deleting %s", path);
#ifdef _WIN32
    if(!DeleteFileA(path)) {
        VL_Log(VL_ERROR, "Could not delete file %s: %s", path, Win32_ErrorMessage(GetLastError()));
        return false;
    }
    return true;
#else
    if(remove(path) < 0) {
        VL_Log(VL_ERROR, "Could not delete file %s: %s", path, strerror(errno));
        return false;
    }
    return true;
#endif // _WIN32
}

VLIBPROC bool sb_ReadEntireFile(const char *path, string_builder *sb)
{
    bool result = true;

    FILE *f = fopen(path, "rb");
    size_t new_count = 0;
    long long m = 0;
    if(f == NULL)                 VL_ReturnDefer(false);
    if(fseek(f, 0, SEEK_END) < 0) VL_ReturnDefer(false);
#ifndef _WIN32
    m = ftell(f);
#else
    m = _ftelli64(f);
#endif
    if(m < 0)                     VL_ReturnDefer(false);
    if(fseek(f, 0, SEEK_SET) < 0) VL_ReturnDefer(false);

    new_count = sb->count + m;
    if(new_count > sb->capacity) {
        sb->items = VL_DECLTYPE_CAST(sb->items)VL_REALLOC(sb->items, new_count);
        Assert(sb->items != NULL && "Buy more RAM lool!!");
        sb->capacity = new_count;
    }

    fread(sb->items + sb->count, m, 1, f);
    if(ferror(f)) {
        // TODO: Afaik, ferror does not set errno. So the error reporting in defer is not correct in this case.
        VL_Log(VL_ERROR, "Could not read file %s", path);
        fclose(f);
        return false;
    }
    sb->count = new_count;

defer:
    if(!result) VL_Log(VL_ERROR, "Could not read file %s: %s", path, strerror(errno));
    if(f) fclose(f);
    return result;
}

VLIBPROC int sb_Appendf(string_builder *sb, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    // NOTE: the new_capacity needs to be +1 because of the null terminator.
    // However, further below we increase sb->count by n, not n + 1.
    // This is because we don't want the sb to include the null terminator. The user can always sb_append_null() if they want it
    da_Reserve(sb, sb->count + n + 1);
    char *dest = sb->items + sb->count;
    va_start(args, fmt);
    vsnprintf(dest, n+1, fmt, args);
    va_end(args);

    sb->count += n;

    return n;
}

VLIBPROC bool sb_PadAlign(string_builder *sb, size_t size)
{
    size_t rem = sb->count%size;
    if(rem == 0) return true;
    for(size_t i = 0; i < size - rem; ++i) {
        da_Append(sb, 0);
    }
    return true;
}

VLIBPROC bool VL_ProcWait(vl_proc proc)
{
    if(proc == VL_INVALID_PROC) return false;

#ifdef _WIN32
    DWORD result = WaitForSingleObject(
                       proc,    // HANDLE hHandle,
                       INFINITE // DWORD  dwMilliseconds
                   );

    if(result == WAIT_FAILED) {
        VL_Log(VL_ERROR, "could not wait on child process: %s", Win32_ErrorMessage(GetLastError()));
        return false;
    }

    DWORD exit_status;
    if(!GetExitCodeProcess(proc, &exit_status)) {
        VL_Log(VL_ERROR, "could not get process exit code: %s", Win32_ErrorMessage(GetLastError()));
        return false;
    }

    if(exit_status != 0) {
        VL_Log(VL_ERROR, "command exited with exit code %lu", exit_status);
        return false;
    }

    CloseHandle(proc);

    return true;
#else
    for(;;) {
        int wstatus = 0;
        if(waitpid(proc, &wstatus, 0) < 0) {
            VL_Log(VL_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
            return false;
        }

        if(WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if(exit_status != 0) {
                VL_Log(VL_ERROR, "command exited with exit code %d", exit_status);
                return false;
            }

            break;
        }

        if(WIFSIGNALED(wstatus)) {
            VL_Log(VL_ERROR, "command process was terminated by signal %d", WTERMSIG(wstatus));
            return false;
        }
    }

    return true;
#endif
}

VLIBPROC int VL__ProcWaitAsync(vl_proc proc, int ms)
{
    if (proc == VL_INVALID_PROC) return false;

#ifdef _WIN32
    DWORD result = WaitForSingleObject(
                       proc,    // HANDLE hHandle,
                       ms       // DWORD  dwMilliseconds
                   );

    if(result == WAIT_TIMEOUT) {
        return 0;
    }

    if(result == WAIT_FAILED) {
        VL_Log(VL_ERROR, "could not wait on child process: %s", Win32_ErrorMessage(GetLastError()));
        return -1;
    }

    DWORD exit_status;
    if(!GetExitCodeProcess(proc, &exit_status)) {
        VL_Log(VL_ERROR, "could not get process exit code: %s", Win32_ErrorMessage(GetLastError()));
        return -1;
    }

    if(exit_status != 0) {
        VL_Log(VL_ERROR, "command exited with exit code %lu", exit_status);
        return -1;
    }

    CloseHandle(proc);

    return 1;
#else
    long ns = ms*1000*1000;
    struct timespec duration = {
        .tv_sec = ns/(1000*1000*1000),
        .tv_nsec = ns%(1000*1000*1000),
    };

    int wstatus = 0;
    pid_t pid = waitpid(proc, &wstatus, WNOHANG);
    if(pid < 0) {
        VL_Log(VL_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
        return -1;
    }

    if(pid == 0) {
        nanosleep(&duration, NULL);
        return 0;
    }

    if(WIFEXITED(wstatus)) {
        int exit_status = WEXITSTATUS(wstatus);
        if(exit_status != 0) {
            VL_Log(VL_ERROR, "command exited with exit code %d", exit_status);
            return -1;
        }

        return 1;
    }

    if(WIFSIGNALED(wstatus)) {
        VL_Log(VL_ERROR, "command process was terminated by signal %d", WTERMSIG(wstatus));
        return -1;
    }

    nanosleep(&duration, NULL);
    return 0;
#endif
}

// Wait until all the processes have finished
VLIBPROC bool VL_ProcsWait(vl_procs procs)
{
    bool ok = true;
    for(size_t i = 0; i < procs.count; ++i) {
        ok = VL_ProcWait(procs.items[i]) && ok;
    }
    return ok;
}

// Wait until all the processes have finished and empty the procs array.
VLIBPROC bool VL_ProcsFlush(vl_procs *procs)
{
    bool ok = VL_ProcsWait(*procs);
    procs->count = 0;
    return ok;
}

VLIBPROC void VL_CmdRender(vl_cmd cmd, string_builder *render)
{
    for(size_t i = 0; i < cmd.count; ++i) {
        const char *arg = cmd.items[i];
        if(arg == NULL) break;
        if(i > 0) sb_AppendCstr(render, " ");
        if(!strchr(arg, ' ')) {
            sb_AppendCstr(render, arg);
        } else {
            da_Append(render, '\'');
            sb_AppendCstr(render, arg);
            da_Append(render, '\'');
        }
    }
}

VLIBPROC bool CmdRun_Opt(vl_cmd_opts opt)
{
    bool result = true;
    vl_fd fdin = VL_INVALID_FD;
    vl_fd fdout = VL_INVALID_FD;
    vl_fd fderr = VL_INVALID_FD;
    vl_fd *optFdin = 0;
    vl_fd *optFdout = 0;
    vl_fd *optFderr = 0;

    size_t max_procs = opt.maxProcs > 0 ? opt.maxProcs : (size_t) VL_GetCountProcs() + 1;

    // TODO: Async
    if(opt.async && max_procs > 0) {
        while(opt.async->count >= max_procs) {
            for(size_t i = 0; i < opt.async->count; ++i) {
                int ret = VL__ProcWaitAsync(opt.async->items[i], 1);
                if(ret < 0) VL_ReturnDefer(false);
                if(ret) {
                    da_RemoveUnordered(opt.async, i);
                    break;
                }
            }
        }
    }

    // TODO: > /dev/null & > nul
    if(opt.stdinPath) {
        fdin = fd_OpenForRead(opt.stdinPath);
        if(fdin == VL_INVALID_FD) VL_ReturnDefer(false);
        optFdin = &fdin;
    }
    if(opt.stdoutPath) {
        fdout = fd_OpenForWrite(opt.stdoutPath);
        if(fdout == VL_INVALID_FD) VL_ReturnDefer(false);
        optFdout = &fdout;
    }
    if(opt.stderrPath) {
        fderr = fd_OpenForWrite(opt.stderrPath);
        if(fderr == VL_INVALID_FD) VL_ReturnDefer(false);
        optFderr = &fderr;
    }
    vl_proc proc = VL__CmdStartProcess(*opt.cmd, optFdin, optFdout, optFderr);

    if(opt.async) {
        if(proc == VL_INVALID_PROC) VL_ReturnDefer(false);
        da_Append(opt.async, proc);
    } else {
        if(!VL_ProcWait(proc)) VL_ReturnDefer(false);
    }

defer:
    if(optFdin)  fd_Close(*optFdin);
    if(optFdout) fd_Close(*optFdout);
    if(optFderr) fd_Close(*optFderr);
    opt.cmd->count = 0;
#if OS_WINDOWS
    opt.cmd->msvc_linkflags = 0;
#endif
    return result;
}

VLIBPROC int VL_GetCountProcs(void)
{
    static int count = 0;
    if(count != 0) return count;
#ifdef _WIN32
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    count = siSysInfo.dwNumberOfProcessors;
#else
    count = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    return count;
}

VLIBPROC char *temp_sprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    Assert(n >= 0);
    char *result = (char*)ArenaPushSize(&ArenaTemp, n + 1, .Alignment = 1);
    va_start(args, fmt);
    vsnprintf(result, n + 1, fmt, args);
    va_end(args);

    return result;
}

VLIBPROC const char *VL_PathName(const char *path)
{
#ifdef _WIN32
    const char *p1 = strrchr(path, '/');
    const char *p2 = strrchr(path, '\\');
    const char *p = (p1 > p2)? p1 : p2;  // NULL is ignored if the other search is successful
    return p ? p + 1 : path;
#else
    const char *p = strrchr(path, '/');
    return p ? p + 1 : path;
#endif // _WIN32
}

VLIBPROC bool VL_Rename(const char *old_path, const char *new_path)
{
    VL_Log(VL_ECHO, "renaming %s -> %s", old_path, new_path);
#ifdef _WIN32
    if(!MoveFileEx(old_path, new_path, MOVEFILE_REPLACE_EXISTING)) {
        VL_Log(VL_ERROR, "could not rename %s to %s: %s", old_path, new_path, Win32_ErrorMessage(GetLastError()));
        return false;
    }
#else
    if(rename(old_path, new_path) < 0) {
        VL_Log(VL_ERROR, "could not rename %s to %s: %s", old_path, new_path, strerror(errno));
        return false;
    }
#endif // _WIN32
    return true;
}

VLIBPROC bool VL_GetLastWriteTime(const char *file, u64 *writeTime)
{
    bool result = true;
    if(!GetLastWriteTime(file, writeTime)) {
#ifdef _WIN32
        VL_Log(VL_ERROR, "Could not get filetime of %s: %s", file, Win32_ErrorMessage(GetLastError()));
#else
        VL_Log(VL_ERROR, "Could not get filetime of %s: %s", file, strerror(errno));
#endif
        result = false;
    }
    return result;
}

VLIBPROC int VL_NeedsRebuild_Impl(const char *output_path, const char **input_paths, size_t input_paths_count)
{
    u64 outputFileTime;
    if(!GetLastWriteTime(output_path, &outputFileTime)) return 1;

    for(size_t i = 0; i < input_paths_count; ++i) {
        const char *input_path = input_paths[i];
        u64 inputFileTime;
        if(!VL_GetLastWriteTime(input_path, &inputFileTime)) return -1;

        // NOTE: if even a single input_path is fresher than output_path that's 100% rebuild
        if(inputFileTime > outputFileTime) return 1;
    }

    return 0;
}

vl_needrebuild_context VL_needsRebuildContext = {0};

VLIBPROC int VL_Needs_C_Rebuild_Impl(const char *output_path, const char **input_paths, size_t input_paths_count)
{
    vl_needrebuild_context *ctx = &VL_needsRebuildContext;
    Assert(ctx->Arena != &ArenaTemp);
    if(ctx->table.nodes.items == 0) {
        ctx->table.nodes.capacity = VL_BUILD_FILETIME_TABLE_SIZE;
        ctx->table.nodes.items = (vl_filetime_node*)VL_REALLOC(0, ctx->table.nodes.capacity*sizeof(vl_filetime_node));
        mem_zero(ctx->table.nodes.items, ctx->table.nodes.capacity*sizeof(vl_filetime_node));
        ctx->table.nodes.count = 0;
    }
    Assert((ctx->table.nodes.capacity & (ctx->table.nodes.capacity-1)) == 0); /* capacity must be power of 2 */

    u64 outputFileTime;
    if(!GetLastWriteTime(output_path, &outputFileTime)) return 1;

    bool result = false;
    string_builder sb = {0};
    vl_file_paths searchPaths = {0};
    vl_file_paths searchPathsDone = {0};

    size_t startTempMark = temp_save();

    u32 hashval;
    da_AppendMany(&searchPaths, input_paths, input_paths_count);
    // TODO: Need some way to know if I explored all dependencies of a specific file
    while(searchPaths.count > 0) {
        char *currPath = (char*)searchPaths.items[0];
        view vPath = view_FromCstr(currPath);

        da_Append(&searchPathsDone, currPath);
        // unordered remove in each iteration
        searchPaths.count--;
        searchPaths.items[0] = searchPaths.items[searchPaths.count];

        VL_BUILD_FILENAME_HASH(vPath, hashval);
        hashval &= ctx->table.nodes.capacity - 1;
        vl_filetime_node *node = &ctx->table.nodes.items[hashval];
        vl_filetime_node prev_ = {.next = node};
        vl_filetime_node *prev = &prev_;
        while(prev->next && !view_Eq(prev->next->file, vPath)) {
            prev = prev->next;
        }

        if(!prev->next) {
            u64 inputFileTime;
            if(!VL_GetLastWriteTime(currPath, &inputFileTime)) return -1;

            if(ctx->freelist) {
                prev->next = ctx->freelist;
                ctx->freelist = ctx->freelist->next;
            } else if(ctx->Arena) {
                prev->next = PushStruct(ctx->Arena, vl_filetime_node);
            } else {
                prev->next = (vl_filetime_node*)VL_REALLOC(0, sizeof(vl_filetime_node));
            }
            prev->next->file = vPath;
            prev->next->time = inputFileTime;
            prev->next->next = 0;
            //prev->next->exploredAllDependencies = false;

            ctx->table.countTimes++;
            if(ctx->table.countTimes > (ctx->table.nodes.capacity >> 1)) {
                /* Resize the hash table, this is pretty slow... */
                vl_filetime_nodelist newNodes;
                newNodes.capacity = ctx->table.nodes.capacity << 1,
                newNodes.items = VL_REALLOC(0, newNodes.capacity*sizeof(vl_filetime_node));
                newNodes.count = 0;

                for(size_t i = 0; i < ctx->table.nodes.count; i++) {
                    if(ctx->table.nodes.items[i].file.count != 0) {
                        vl_filetime_node *oldnode = &ctx->table.nodes.items[i];
                        for(; oldnode; oldnode = oldnode->next) {
                            VL_BUILD_FILENAME_HASH(oldnode->file, hashval);
                            hashval &= newNodes.capacity - 1;
                            
                            vl_filetime_node newPrev_ = {.next = &newNodes.items[hashval]};
                            vl_filetime_node *newPrev = &newPrev_;
                            if(newPrev->next->file.count != 0) {
                                while(newPrev->next) prev = prev->next;
                                if(ctx->freelist) {
                                    prev->next = ctx->freelist;
                                    ctx->freelist = ctx->freelist->next;
                                } else if(ctx->Arena) {
                                    prev->next = PushStruct(ctx->Arena, vl_filetime_node);
                                } else {
                                    prev->next = (vl_filetime_node*)VL_REALLOC(0, sizeof(vl_filetime_node));
                                }
                            } else {
                                newNodes.count++;
                            }
                            prev->next->file = oldnode->file;
                            prev->next->time = oldnode->time;
                            prev->next->next = 0;

                            vl_filetime_node *nextFree = ctx->freelist;
                            ctx->freelist = oldnode;
                            ctx->freelist->next = nextFree;
                        }
                    }
                }

                VL_FREE(ctx->table.nodes.items);
                ctx->table.nodes = newNodes;
            }
        }

        if(prev->next->time > outputFileTime) {
            result = true;
            break;
        }
        
        //if(!node->exploredAllDependencies) {
            sb.count = 0;
            if(!sb_ReadEntireFile(vPath.items, &sb)) continue;

            sb_AppendNull(&sb);

            // TODO: Check if smarter parsing, where we don't try to do anything when it's commented out (in a multiline comment) is any better/quicker (I don't think so)
            char *ptr = sb.items;
            while(ptr) {
                // TODO: Use viclib's view parsing for this when view_Contains is redone
                ptr = strstr(ptr, "\n#include");
                if(!ptr) break;
                ptr = strpbrk(ptr + strlen("\n#include"), "\"<"); /* find first '<' or '"' character */
                /* very unlikely, but there is the case where the include is a macro constant. See how viclib.h is included in vl_build.h */
                if(!ptr) continue;

                char closeChar = *ptr == '"' ? '"' : '>';
                char *filename = ptr + 1;
                ptr = strchr(filename, closeChar);
                if(!ptr) break;
                filename = temp_strndup(filename, ptr - filename);

                /* 1. Check in same directory as this file (the one with the #include)
                 * 2. Check in the same directory as currPath
                 * 3. Check in ctx->includePaths[i]
                 */
                bool found = false;
                LinearSearch(&searchPathsDone, filename, !strcmp);
                if(found) continue;

                if(VL_FileExists(filename)) {
                    // Found in 1.
                    LinearSearch(&searchPaths, filename, !strcmp);
                    if(!found) da_Append(&searchPaths, filename);
                    continue;
                }

                char *contiguousPath = currPath;
                size_t tempMark = temp_save();
                char *lastSlash = contiguousPath + strlen(contiguousPath) - 1;
                while(lastSlash > contiguousPath && *lastSlash != '/') lastSlash--;
                contiguousPath =
                    temp_sprintf("%.*s/%s", (int)(lastSlash - contiguousPath), contiguousPath, filename);
                LinearSearch(&searchPathsDone, contiguousPath, !strcmp);
                if(found) continue;

                if(VL_FileExists(contiguousPath)) {
                    // Found in 2.
                    LinearSearch(&searchPaths, contiguousPath, !strcmp);
                    if(!found) da_Append(&searchPaths, contiguousPath);
                    continue;
                }

                if(ctx->includePaths) {
                    for(size_t incIdx = 0; incIdx < ctx->includePaths->count; incIdx++) {
                        char *path = temp_sprintf("%s/%s", ctx->includePaths->items[incIdx], filename);
                        if(VL_FileExists(path)) {
                            LinearSearch(&searchPathsDone, path, !strcmp);
                            temp_rewind(tempMark);
                            if(found) {
                                break;
                            } else {
                                // Found in 3.
                                path = temp_sprintf("%s/%s", ctx->includePaths->items[incIdx], filename);
                                LinearSearch(&searchPaths, path, !strcmp);
                                if(!found) da_Append(&searchPaths, path);
                                else temp_rewind(tempMark);
                                found = true;
                                break;
                            }
                        }
                    }
                }
                if(!found) {
                    /* Add it to already done search paths if it doesn't get found since it's most likely a 
                       standard libary include: e.g. math.h */
                    da_Append(&searchPathsDone, filename);
                }
            }
        //}
    }

    temp_rewind(startTempMark);

    da_Free(searchPathsDone);
    da_Free(searchPaths);
    da_Free(sb);

    return result;
}

/* returns: 0 - does not exist. 1 - exists. -1 - error while checking */
VLIBPROC int VL_FileExists(const char *path)
{
#if _WIN32
    // TODO: distinguish between "does not exists" and other errors
    DWORD dwAttrib = GetFileAttributesA(path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES;
#else
    struct stat statbuf;
    if(stat(path, &statbuf) < 0) {
        if(errno == ENOENT) return 0;
        VL_Log(VL_ERROR, "Could not check if file %s exists: %s", path, strerror(errno));
        return -1;
    }
    return 1;
#endif
}

VLIBPROC const char *VL_temp_GetCurrentDir(void)
{
#ifdef _WIN32
    DWORD nBufferLength = GetCurrentDirectory(0, NULL);
    if (nBufferLength == 0) {
        VL_Log(VL_ERROR, "could not get current directory: %s", Win32_ErrorMessage(GetLastError()));
        return NULL;
    }

    char *buffer = (char*) temp_alloc(nBufferLength, .Alignment = 1);
    if (GetCurrentDirectory(nBufferLength, buffer) == 0) {
        VL_Log(VL_ERROR, "could not get current directory: %s", Win32_ErrorMessage(GetLastError()));
        return NULL;
    }

    return buffer;
#else
    char *buffer = (char*) temp_alloc(PATH_MAX, .Alignment = 1);
    if (getcwd(buffer, PATH_MAX) == NULL) {
        VL_Log(VL_ERROR, "could not get current directory: %s", strerror(errno));
        return NULL;
    }

    return buffer;
#endif // _WIN32
}

struct vl__pushd_buf_type VL__pushDirectoryBuffer;

VLIBPROC bool VL_Pushd(const char *path)
{
    static bool initialized = false;
    if(!initialized) {
        initialized = true;
        VL__pushDirectoryBuffer.items[0] = temp_strdup(VL_temp_GetCurrentDir());
        VL__pushDirectoryBuffer.count = 1;
    }

    AssertMsgAlways((VL__pushDirectoryBuffer.count+1) < VL_PUSHD_BUF_MAX, "Increase the size of the pushd buffer (VL_PUSHD_BUF_MAX)");
    bool ok = VL_SetCurrentDir(path);
    if(ok) {
        VL__pushDirectoryBuffer.items[VL__pushDirectoryBuffer.count] = 
            temp_sprintf("%s/%s", VL__pushDirectoryBuffer.items[VL__pushDirectoryBuffer.count - 1], path);
        VL__pushDirectoryBuffer.count++;
    } else {
#if defined(_WIN32)
        VL_Log(VL_ERROR, "could not set current directory to %s: %s", path, Win32_ErrorMessage(GetLastError()));
#else
        VL_Log(VL_ERROR, "could not set current directory to %s: %s", path, strerror(errno));
#endif
    }
    return ok;
}

VLIBPROC bool VL_Popd(void)
{
    AssertMsg(VL__pushDirectoryBuffer.count > 1, "Need to do pushd before popd");
    const char *path = VL__pushDirectoryBuffer.items[VL__pushDirectoryBuffer.count - 2];
    bool ok = VL_SetCurrentDir(path);
    if(ok) VL__pushDirectoryBuffer.count--;
    else {
#if defined(_WIN32)
        VL_Log(VL_ERROR, "could not set current directory to %s: %s", path, Win32_ErrorMessage(GetLastError()));
#else
        VL_Log(VL_ERROR, "could not set current directory to %s: %s", path, strerror(errno));
#endif
    }
    return ok;
}

VLIBPROC char *VL_temp_DirName(const char *path)
{
#ifndef _WIN32
    // Stolen from the musl's implementation of dirname.
    // We are implementing our own one because libc vendors cannot agree on whether dirname(3)
    // modifies the path or not.
    if(!path || !*path) return ".";
    size_t i = strlen(path) - 1;
    for(; path[i] == '/'; i--) if(!i) return "/";
    for(; path[i] != '/'; i--) if(!i) return ".";
    for(; path[i] == '/'; i--) if(!i) return "/";
    return temp_strndup(path, i + 1);
#else
    if(!path) path = ""; // Treating NULL as empty.
    char *drive = temp_alloc(_MAX_DRIVE, .Alignment = 1);
    char *dir   = temp_alloc(_MAX_DIR, .Alignment = 1);
    // https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2010/8e46eyt7(v=vs.100)
    errno_t ret = _splitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
    Assert(ret == 0);
    return temp_sprintf("%s%s", drive, dir);
#endif // _WIN32
}

VLIBPROC char *VL_temp_FileName(const char *path)
{
#ifndef _WIN32
    // Stolen from the musl's implementation of dirname.
    // We are implementing our own one because libc vendors cannot agree on whether basename(3)
    // modifies the path or not.
    if(!path || !*path) return ".";
    size_t i = strlen(path)-1;
    char *s = temp_strndup(path, i);
    for(; i&&s[i]=='/'; i--) s[i] = 0;
    for(; i&&s[i-1]!='/'; i--);
    return s+i;
#else
    if(!path) path = ""; // Treating NULL as empty.
    char *fname = temp_alloc(_MAX_FNAME, .Alignment = 1);
    char *ext = temp_alloc(_MAX_EXT, .Alignment = 1);
    // https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2010/8e46eyt7(v=vs.100)
    errno_t ret = _splitpath_s(path, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);
    Assert(ret == 0);
    return temp_sprintf("%s%s", fname, ext);
#endif // _WIN32
}

VLIBPROC char *VL_temp_FileExt(const char *path)
{
#ifndef _WIN32
    return strrchr(VL_temp_FileName(path), '.');
#else
    if(!path) path = ""; // Treating NULL as empty.
    char *ext = temp_alloc(_MAX_EXT, .Alignment = 1);
    // https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2010/8e46eyt7(v=vs.100)
    errno_t ret = _splitpath_s(path, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT);
    Assert(ret == 0);
    return ext;
#endif // _WIN32
}

VLIBPROC char *VL_temp_RunningExecutablePath(void)
{
#if defined(__linux__)
    char buf[4096];
    int length = readlink("/proc/self/exe", buf, ArrayLen(buf));
    if(length < 0) return "";
    return temp_strndup(buf, length);
#elif defined(_WIN32)
    char buf[MAX_PATH];
    int length = GetModuleFileNameA(NULL, buf, MAX_PATH);
    return temp_strndup(buf, length);
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t size = ArrayLen(buf);
    if(_NSGetExecutablePath(buf, &size) != 0) return "";
    int length = strlen(buf);
    return temp_strndup(buf, length);
#else
    fprintf(stderr, LOC_STR": TODO: %s is not implemented for this platform\n", __FUNCTION__);
    return "";
#endif
}

VLIBPROC void VL_cc_Opt(struct compiler_info_opts opt)
{
    switch(opt.cc) {
        case CCompiler_GCC: {
            cmd_Append(opt.cmd, "cc");
        } break;
        case CCompiler_Clang: {
            cmd_Append(opt.cmd, "clang");
        } break;
        case CCompiler_MSVC: {
            cmd_Append(opt.cmd, "cl.exe");
        } break;
    }
}

VLIBPROC void VL_ccWarnings_Opt(struct compiler_info_opts opt)
{
    switch(opt.cc) {
        case CCompiler_GCC:
        case CCompiler_Clang: {
            cmd_Append(opt.cmd, "-Wall", "-Wextra");
        } break;
        case CCompiler_MSVC: {
            cmd_Append(opt.cmd, "-W4", "-nologo", "-D_CRT_SECURE_NO_WARNINGS");
        } break;
    }
}

VLIBPROC void VL_ccWarningsAsErrors_Opt(struct compiler_info_opts opt)
{
    switch(opt.cc) {
        case CCompiler_GCC:
        case CCompiler_Clang: {
            cmd_Append(opt.cmd, "-Werror");
        } break;
        case CCompiler_MSVC: {
            cmd_Append(opt.cmd, "-WX");
        } break;
    }
}

VLIBPROC void VL_ccOutput_Opt(struct compiler_info_opts opt, const char *output)
{
    Assert(output);
    switch(opt.cc) {
        case CCompiler_GCC:
        case CCompiler_Clang: {
            cmd_Append(opt.cmd, "-o", output);
        } break;
        case CCompiler_MSVC: {
            cmd_Append(opt.cmd, temp_sprintf("/Fe:%s", output));
        } break;
    }
}

VLIBPROC void VL_ccDebug_Opt(struct compiler_info_opts opt)
{
    switch(opt.cc) {
        case CCompiler_GCC:
        case CCompiler_Clang: {
            cmd_Append(opt.cmd, "-g");
        } break;
        case CCompiler_MSVC: {
            cmd_Append(opt.cmd, "-Zi");
        } break;
    }
}

VLIBPROC void VL_ccLibs_Opt(struct compiler_info_opts opt, const char **libs, size_t libcount)
{
#if OS_WINDOWS
    if(opt.cc == CCompiler_MSVC && !opt.cmd->msvc_linkflags) {
        cmd_Append(opt.cmd, "/link");
        opt.cmd->msvc_linkflags = true;
    }
#endif

    for(size_t i = 0; i < libcount; i++) {
        switch(opt.cc) {
            case CCompiler_GCC:
            case CCompiler_Clang: {
                cmd_Append(opt.cmd, "-l", libs[i]);
            } break;
            case CCompiler_MSVC: {
                cmd_Append(opt.cmd, temp_sprintf("%s.lib", libs[i]));
            } break;
        }
    }
}

VLIBPROC void VL_ccLibpath_Opt(struct compiler_info_opts opt, const char *libpath)
{
#if OS_WINDOWS
    if(opt.cc == CCompiler_MSVC && !opt.cmd->msvc_linkflags) {
        cmd_Append(opt.cmd, "/link");
        opt.cmd->msvc_linkflags = true;
    }
#endif

    switch(opt.cc) {
        case CCompiler_GCC:
        case CCompiler_Clang: {
            cmd_Append(opt.cmd, "-L", libpath);
        } break;
        case CCompiler_MSVC: {
            cmd_Append(opt.cmd, temp_sprintf("/libpath:%s", libpath));
        } break;
    }
}

#if OS_WINDOWS
static void Win32_CmdQuote(vl_cmd cmd, string_builder *quoted)
{
    for(size_t i = 0; i < cmd.count; ++i) {
        const char *arg = cmd.items[i];
        if(arg == NULL) break;
        size_t len = strlen(arg);
        if(i > 0) da_Append(quoted, ' ');
        if(len != 0 && NULL == strpbrk(arg, " \t\n\v\"")) {
            // no need to quote
            da_AppendMany(quoted, arg, len);
        } else {
            // we need to escape:
            // 1. double quotes in the original arg
            // 2. consequent backslashes before a double quote
            size_t backslashes = 0;
            da_Append(quoted, '\"');
            for(size_t j = 0; j < len; ++j) {
                char x = arg[j];
                if(x == '\\') {
                    backslashes += 1;
                } else {
                    if(x == '\"') {
                        // escape backslashes (if any) and the double quote
                        for(size_t k = 0; k < 1+backslashes; ++k) {
                            da_Append(quoted, '\\');
                        }
                    }
                    backslashes = 0;
                }
                da_Append(quoted, x);
            }
            // escape backslashes (if any)
            for(size_t k = 0; k < backslashes; ++k) {
                da_Append(quoted, '\\');
            }
            da_Append(quoted, '\"');
        }
    }
}
#endif

static vl_proc VL__CmdStartProcess(vl_cmd cmd, vl_fd *fdin, vl_fd *fdout, vl_fd *fderr)
{
    if(cmd.count < 1) {
        VL_Log(VL_ERROR, "Could not run empty command");
        return VL_INVALID_PROC;
    }
#if OS_WINDOWS
    cmd.msvc_linkflags = false;
#endif

    string_builder sb = {0};
    VL_CmdRender(cmd, &sb);
    sb_AppendNull(&sb);
    VL_Log(VL_INFO, "CMD: %s", sb.items);
    sb_Free(sb);
    memset(&sb, 0, sizeof(sb));

#if OS_WINDOWS
    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    // TODO: check for errors in GetStdHandle
    siStartInfo.hStdError = fderr ? *fderr : GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = fdout ? *fdout : GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = fdin ? *fdin : GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    Win32_CmdQuote(cmd, &sb);
    sb_AppendNull(&sb);
    BOOL bSuccess = CreateProcessA(NULL, sb.items, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);
    sb_Free(sb);

    if(!bSuccess) {
        VL_Log(VL_ERROR, "Could not create child process for %s: %s", cmd.items[0], Win32_ErrorMessage(GetLastError()));
        return VL_INVALID_PROC;
    }

    CloseHandle(piProcInfo.hThread);

    return piProcInfo.hProcess;
#else
    pid_t cpid = fork();
    if(cpid < 0) {
        VL_Log(VL_ERROR, "Could not fork child process: %s", strerror(errno));
        return VL_INVALID_PROC;
    }

    if(cpid == 0) {
        if(fdin) {
            if(dup2(*fdin, STDIN_FILENO) < 0) {
                VL_Log(VL_ERROR, "Could not setup stdin for child process: %s", strerror(errno));
                exit(1);
            }
        }

        if(fdout) {
            if(dup2(*fdout, STDOUT_FILENO) < 0) {
                VL_Log(VL_ERROR, "Could not setup stdout for child process: %s", strerror(errno));
                exit(1);
            }
        }

        if(fderr) {
            if(dup2(*fderr, STDERR_FILENO) < 0) {
                VL_Log(VL_ERROR, "Could not setup stderr for child process: %s", strerror(errno));
                exit(1);
            }
        }

        // NOTE: This leaks a bit of memory in the child process.
        // But do we actually care? It's a one off leak anyway...
        vl_cmd cmdNull = {0};
        da_AppendMany(&cmdNull, cmd.items, cmd.count);
        cmd_Append(&cmdNull, NULL);

        if(execvp(cmd.items[0], (char * const*) cmdNull.items) < 0) {
            VL_Log(VL_ERROR, "Could not exec child process for %s: %s", cmd.items[0], strerror(errno));
            exit(1);
        }
        Assert(!"unreachable");
    }

    return cpid;
#endif
}

VLIBPROC void VL__GoRebuildUrself(int argc, char **argv, const char **src_paths, size_t path_count)
{
    Assert(argc > 0);
    const char *bin_path = *argv;
    argc--;
    argv++;
#ifdef _WIN32
    // On Windows executables almost always invoked without extension, so
    // it's ./nob, not ./nob.exe. For renaming the extension is a must.
    if(!view_EndWith(view_FromCstr(bin_path), VIEW_STATIC(".exe"))) {
        bin_path = temp_sprintf("%s.exe", bin_path);
    }
#endif

    int rebuild_is_needed = VL_NeedsRebuild_Impl(bin_path, src_paths, path_count);
    if(rebuild_is_needed < 0) exit(1);
    if(!rebuild_is_needed) return;

    vl_cmd cmd = {0};

    const char *old_bin_path = temp_sprintf("%s.old", bin_path);

    if(!VL_Rename(bin_path, old_bin_path)) exit(1);
    cmd_Append(&cmd, VL_REBUILD_URSELF(bin_path, src_paths[0]));
    if(!CmdRun(&cmd)) {
        VL_Rename(old_bin_path, bin_path);
        exit(1);
    }
#ifdef VL_EXPERIMENTAL_DELETE_OLD
    // TODO: this is an experimental behavior behind a compilation flag.
    // Once it is confirmed that it does not cause much problems on both POSIX and Windows
    // we may turn it on by default.
    // NOTE: Pretty sure this won't work on windows
    VL_DeleteFile(old_binary_path);
#endif

    cmd_Append(&cmd, bin_path);
    da_AppendMany(&cmd, argv, argc);
    if(!CmdRun(&cmd)) exit(1);
    exit(0);
}

// minirent.h SOURCE BEGIN ////////////////////////////////////////
#if defined(_WIN32)
struct DIR
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    struct dirent *dirent;
};

VLIBPROC DIR *opendir(const char *dirpath)
{
    Assert(dirpath);

    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\*", dirpath);

    DIR *dir = (DIR*)VL_REALLOC(NULL, sizeof(DIR));
    memset(dir, 0, sizeof(DIR));

    dir->hFind = FindFirstFile(buffer, &dir->data);
    if(dir->hFind == INVALID_HANDLE_VALUE) {
        // TODO: opendir should set errno accordingly on FindFirstFile fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        goto fail;
    }

    return dir;

fail:
    if(dir) {
        VL_FREE(dir);
    }

    return NULL;
}

VLIBPROC struct dirent *readdir(DIR *dirp)
{
    Assert(dirp);

    if(dirp->dirent == NULL) {
        dirp->dirent = (struct dirent*)VL_REALLOC(NULL, sizeof(struct dirent));
        memset(dirp->dirent, 0, sizeof(struct dirent));
    } else {
        if(!FindNextFile(dirp->hFind, &dirp->data)) {
            if(GetLastError() != ERROR_NO_MORE_FILES) {
                // TODO: readdir should set errno accordingly on FindNextFile fail
                // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
                errno = ENOSYS;
            }

            return NULL;
        }
    }

    memset(dirp->dirent->d_name, 0, sizeof(dirp->dirent->d_name));

    strncpy(
        dirp->dirent->d_name,
        dirp->data.cFileName,
        sizeof(dirp->dirent->d_name) - 1);

    return dirp->dirent;
}

VLIBPROC int closedir(DIR *dirp)
{
    Assert(dirp);

    if(!FindClose(dirp->hFind)) {
        // TODO: closedir should set errno accordingly on FindClose fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        return -1;
    }

    if (dirp->dirent) {
        VL_FREE(dirp->dirent);
    }
    VL_FREE(dirp);

    return 0;
}
#endif // _WIN32
// minirent.h SOURCE END ////////////////////////////////////////

#endif // VL_BUILD_IMPLEMENTATION
#endif // VL_BUILD_H