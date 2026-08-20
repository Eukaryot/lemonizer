# Lemonizer
A custom tool for translating 68K code to lemonscript.

Please be aware that this is meant as additional tool for people with prior knowledge of ROM hacking and/or 68K code for the Sega Mega Drive / Genesis. It's not a simple decompiler that magically creates a working Lemonscript / Oxygen Engine project from a game ROM. See the limitations listed below for details.

# How to run
As a Windows user, you can use the latest releases. Or in any case, you can try building Lemonizer yourself, see below.
Have a look at the "how-to-use.txt" for details on what is needed to setup and run the tool.

# How to build
Unfortunately, as of now, there's only build support for Windows.
To build yourself:
- This requires Visual Studio 2022 or newer, either Community or Professional edition.
- First get a copy of the [sonic3air](https://github.com/Eukaryot/sonic3air) repository. This will contain all the dependencies required for the build.
- Setup all the dependencies and build sonic3air once as described in "Oxygen/sonic3air/build/_vstudio/how-to-build.txt".
- When that is done, place a copy of the lemonizer repo into a new directory "Oxygen/lemonizer".
- Open "Oxygen/lemonizer/build/_vstudio" and launch "lemonizer.sln" there.
- Build and run with Visual Studio.

# Limitations
The code translation has quite a few limitations that will make manual touch-ups of the gerenated code a necessity:
- There might be single 68K opcodes that simply can't be translated as they're not (yet?) supported.
- The tool will try to create structured code (e.g. if-else-blocks) from the 68K code's jumps, but this generally can't resolve all jumps, so you will likely need to resolve the remaining ones yourself.
- Some opcode like comparisons won't always get fully resolved, when the actual condition is not clear from the context. Instead, it will then create something like "if (!equal())" or "if (aboveEqual.u())", which need to be taken care of.
- In some cases, generated functions may have multiple entry points. Make sure to manually resolve all of them by splitting the function into multiple functions with their own address hooks.
- Interactions with the VDP graphics chip or Z80 co-processor (including all reads and writes to addresses in the form 0xa????? and 0xc?????) are simply translated, but these won't work in Oxygen Engine. Instead, they need to be rewritten to use the engine's functionality. However, it highly depends on the use-case, which functions need to be used, so this can't really be generalized.
- And of course, the tool probably has a fair bunch of yet unknown bugs remaining...
