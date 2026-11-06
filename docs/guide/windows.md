# Windows

Nirea runs on Windows with a MinGW-w64 toolchain: GCC or Clang.

## Build the compiler

Install CMake and a MinGW-w64 toolchain: GCC (for example from MSYS2 or WinLibs) or Clang (for example llvm-mingw). With GCC:

```sh
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
ctest --test-dir build -j8
```

The compiler is `build\nirea.exe`. It needs GCC or Clang on `PATH` to build programs too, because it compiles the C it generates with them. Microsoft's C compiler (`cl.exe`) isn't supported.

## Use the compiler

Executables get `.exe`, and paths can use `\`. In Command Prompt:

```bat
REM build and run a single file
build\nirea.exe run Hello.ni

REM build Hello.exe in the current folder, then run it
build\nirea.exe build Hello.ni
Hello.exe

REM build bin\hello.exe
build\nirea.exe build Hello.ni -o bin\hello

REM build a project into C:\projects\garden\target\nirea-app.exe
build\nirea.exe build C:\projects\garden

REM use Clang instead of GCC for one build
build\nirea.exe run Hello.ni --cc clang
```

PowerShell takes the same commands without the `REM` lines, except that a program in the current folder is run as `.\Hello.exe`. To use Clang for every build, set `CC`: `set CC=clang` in Command Prompt, or `$env:CC = "clang"` in PowerShell. To call the compiler as just `nirea`, add the `build` folder to `PATH`, for example with `set PATH=%CD%\build;%PATH%` in Command Prompt.

---

[← Calling C](c-interop.md) · [Contents](README.md)
