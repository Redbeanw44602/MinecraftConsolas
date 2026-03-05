# MinecraftConsolas

A certain block game, based on https://github.com/smartcmd/MinecraftConsoles ([`d26f70a`](https://github.com/smartcmd/MinecraftConsoles/commit/d26f70a0cb33b7410ae959a3597b53b5d3e5090f))

### What's changed?

 - Migrate msbuild to xmake.
 - Only keep the files needed for building on Windows x64.
 - Directory structure (`vcxproj.filters`) has been restored and manually optimized.
 - IWYU has been run on the entire codebase.
 - PCH abuse has been eliminated (`stdafx.h` no longer exists).
 - Remove all statements `using namespace std;`.
 - Remove some silly macros (such as AUTO_VAR...).
 - `.clang-format` has been applied throughout the codebase.
 - It can be compiled under Linux (via [msvc-wine](https://github.com/mstorsjo/msvc-wine) + clang).

### What's next?

 - I would like to add native Linux support (SDL3).
