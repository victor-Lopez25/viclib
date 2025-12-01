# VicLib

Header-only library which does some basic stuff you might want in a lot of programs

viclib.h includes:
 - String view implementation (view_* functions)
 - Assertions
 - Simple memory functions (mem_copy, mem_zero, mem_compare)
 - Arena implementation
 - Some file operations (filetime, read/write entirefile, getfiletype)
 - Sort() which performs an introsort

vl_build.h includes:
 - viclib.h since it depends on it
 - string builder implementation (string_builder)
 - list implementation (da_*)
 - File operations (read, write, copy, delete, rename)
 - Directory operations (get cwd, set cwd, pushd, popd, readdir, copy directory recursively)
 - Processes, in async too (cmd*)
 - Some filepath operations
 - helpers to use any c compiler (VL_cc*) (gcc, clang, msvc are supported)
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

### WARNING: preferably use latest release and not a commit to main, since it could be broken

## Usage:
### Quick start
viclib.h:
```c
// include any stuff you might need for viclib before including viclib.h
#include <stdio.h>
#include <stdlib.h>

#define VICLIB_IMPLEMENTATION
#include "viclib.h"

int main()
{
    char *file = "somefile.txt";
    size_t filesize;
    // since stdio.h and stdlib.h have been included before viclib.h, this will call the stdlib version of ReadEntireFile
    char *data = ReadEntireFile(file, &filesize);
    if(!data) {
        fprintf(stderr, "Could not read file '%s': %d\n", file, ErrorNumber); // ErrorNumber is defined in viclib.h
        return 1;
    }
    printf("%.*s\n", (int)filesize, data); // print the entire file
    return 0;
}
```
What if you didn't include `stdio.h` and `stdlib.h`? ReadEntireFile would become an assert(false) with a message saying you need to include some file api.

vl_build.h:
```c
// vl_build.h includes viclib.h, define VICLIB_PATH if it's not "viclib.h"
#define VL_BUILD_IMPLEMENTATION
#include "vl_build.h"

int main(int argc, char **argv)
{
    // rebuilds this file and rerruns it if needed, add extra files to the macro if you need to test if multiple files have been modified
    VL_GO_REBUILD_URSELF(argc, argv);

    vl_cmd cmd = {0};
    VL_cc(&cmd); // chooses the compiler you used to compile this by default
    VL_ccOutput(&cmd, "test" VL_EXE_EXTENSION);
    cmd_Append(&cmd, "src/main.c");
    VL_ccWarnings(&cmd);

    // runs the command stored in cmd and resets cmd
    if(!CmdRun(&cmd)) return 1;

    return 0;
}
```

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

### Licences
viclib.h is licenced under MIT, as shown in the file itself. You are free to change the name of the file if you modify it, just keep the license as "original viclib license by Victor Lopez:\n>mit license here<".

[vl_build.h]() © 2024 by [Víctor López Cortés](https://github.com/victor-Lopez25) is licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)
