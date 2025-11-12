# VicLib

Header-only library which does some basic stuff you might want in a lot of programs

viclib.h includes:
 - String view implementation (view_* functions)
 - Assertions
 - Simple memory functions (mem_copy, mem_zero, mem_compare)
 - Arena implementation
 - ReadEntireFile and ReadFileChunk when using stdlib or windows.h
 - Sort() which performs an introsort

vl_build.h includes:
 - viclib.h since it depends on it
 - string builder implementation (string_builder)
 - list implementation (da_*)
 - File operations (read, write, copy, delete, rename)
 - Directory operations (get cwd, set cwd, pushd, popd, readdir, copy directory recursively)
 - Processes, in async too (cmd*)
 - Some filepath operations
 - helpers to use any c compiler (VL_cc*)
 - NOB_GO_REBUILD_URSELF technology (tm)

To download the header only libs:
```console
wget -O viclib.h https://github.com/victor-Lopez25/viclib/raw/refs/heads/main/viclib.h
```
vl_build.h depends on viclib.h
```console
wget -O viclib.h https://github.com/victor-Lopez25/viclib/raw/refs/heads/main/viclib.h
wget -O vl_build.h https://github.com/victor-Lopez25/viclib/raw/refs/heads/main/vl_build.h
```

## WARNING: This library is unfinished, so don't expect great things

## Usage:
### Defines

To have any of these take effect, you must define them _before_ including this file
 - VICLIB_IMPLEMENTATION: If you want to have the implementation (only in one file)
 - READ_ENTIRE_FILE_MAX: If you want to have a max file read size, default is 0xFFFFFFFF (4GB)
 - QUIET_ASSERT: If you want the assertions to add a breakpoint but not print
 - RELEASE_MODE: Have some stuff work faster, right now, assertions get compiled out when this is defined
 - VICLIB_PROC: Define to 'static' or some kind of export as needed
 - VICLIB_TEMP_SIZE: ArenaTemp size, default is 4\*1024\*1024 bytes
 - VICLIB_NO*: If you want to remove parts of the library:
   - VICLIB_NO_TEMP_ARENA: remove ArenaTemp
   - VICLIB_NO_FILE_IO: remove any file IO functions. Useful for when you already have some other library that does file IO (for example SDL -> SDL_LoadFile)
   - VICLIB_NO_SORT: remove Sort and all functions used by it
Check ErrorNumber when errors occur.

### Many thanks to the inspirations for this library:
 - Mr4th's 4ed_base_types.h - https://mr-4th.itch.io/4coder (find the file in 'custom' directory)
 - stb header-only libraries - https://github.com/nothings/stb
 - tsoding's string view implementation - https://github.com/tsoding/sv
 - tsoding's nobuild - https://github.com/tsoding/nob.h

You are free to change the name of the file if you modify it, just keep the license as "original viclib license by Victor Lopez:\n>mit license here<".
