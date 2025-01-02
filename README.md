# VicLib

Header-only library which does some basic stuff you might want in a lot of programs

## Usage:
### Defines

To have any of these take effect, you must define them _before_ including this file

 - BASE_TYPES_IMPLEMENTATION if you want to have the implementation.
 - READ_ENTIRE_FILE_MAX if you want to have a max file read size for the ReadEntireFile function. It'll default to 0xFFFFFFFF.
 - QUIET_ASSERT if you want the assertions to only crash instead of add a breakpoint and print.
 - RELEASE_MODE to have some stuff work faster, right now, assertions get compiled out when this is defined.

Check ErrorNumber when errors occur.
WARNING I'm pretty sure doing it like this doesn't let you see what the error was when using multithreading when an error happens at the same time in different threads, but this should not happen since errors shouldn't occur 99% of the time anyway. I will however change this if I see it's not great.
Right now, only std_ReadEntireFile sets this.

### Many thanks to the inspirations for this library:
 - Mr4th's 4ed_base_types.h - https://mr-4th.itch.io/4coder (find the file in 'custom' directory)
 - stb header-only libraries - https://github.com/nothings/stb
 - tsoding's string view implementation - https://github.com/tsoding/sv

You are free to change the name of the file if you modify it, just keep the license as "original viclib license by Victor Lopez:\n>mit license here<".
