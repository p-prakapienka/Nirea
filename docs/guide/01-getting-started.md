# 1. Getting started

## Build the compiler

You need CMake 3.16 or later, and GCC or Clang.

```sh
git clone https://github.com/p-prakapienka/Nirea.git
cd Nirea
cmake -S . -B build
cmake --build build
ctest --test-dir build -j8      # optional: run the test suite
```

The compiler is `build/nirea`. To call it as just `nirea`, add the `build` folder to your `PATH`. On Windows, see [Windows](12-windows.md).

## Your first program

Save this as `Hello.ni`:

```nirea
void main() {
    println("Hello, Nirea!");
}
```

Then build and run it:

```sh
nirea run Hello.ni              # prints Hello, Nirea!
```

Every program has one `main` function, written `void main()` or `int main()`. The value `int main()` returns is the program's exit status.

## Commands

```sh
nirea run Hello.ni              # build and run a single file
nirea build Hello.ni -o hello   # build an executable
nirea build path/to/project     # build a project into target/
nirea check path/to/project     # check without building
```

When you build a single file, the compiler also loads the files it imports. A project keeps its sources in `src/main/nirea`. `nirea build` writes the generated C to `target/generated-c/` and the executable to `target/nirea-app`. Without a file or folder, the commands use the project in the current directory. [Projects](10-projects.md) explains the layout.

Options:

- `-o FILE` sets the executable path.
- `--emit-c DIR` keeps the generated C in `DIR`.
- `--cc COMPILER` selects the C compiler. The default is `$CC`, then `cc`; on Windows, `$CC`, then `gcc`, then `clang`.
- `-lLIB` and `-LDIR` are passed to the C compiler, for example to link C libraries used through `extern "C"` ([Calling C](11-c-interop.md)).
- `--version` prints the compiler version.

## Compile errors

The compiler checks all of Nirea's rules before it generates any C. Errors name the file, line and column, say what is wrong and, where possible, how to fix it:

```text
Main.ni:3:5: error: 'count' can't be used here: it has no value yet; assign one to it first
Main.ni:2:9: note: 'count' is declared here without a value
```

Some errors can only be found while the program runs, such as integer overflow or an index outside an array. A runtime error stops the program at once, with a message and exit status 101:

```text
nirea: runtime error: integer overflow at Main.ni:3:8
```

---

[Contents](README.md) · [2. Basics →](02-basics.md)
