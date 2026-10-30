# Nirea

Nirea is a statically typed, memory-safe, native programming language for systems programming and general-purpose development. It pairs Java-inspired syntax with a C-inspired programming model.

```nirea
struct Vec2 {
    float x;
    float y;

    float lengthSquared() {
        return x * x + y * y;
    }
}

void main() {
    println("Hello, Nirea!");
}
```

## Key ideas

- Free functions, structs with methods, interfaces and namespaces. No classes or inheritance.
- Exclusive ownership checked by the compiler. No garbage collector and no user-visible raw pointers.
- Ahead-of-time compilation to native code, with direct interoperability with C libraries.
- A small language core, with further functionality in optional libraries.

## Status

Nirea is at an early stage. The first compiler, `nirea`, implements version 0.8 of the specification. It translates Nirea to C and builds native executables with GCC or Clang.

## Building the compiler

You need CMake 3.16 or later and GCC or Clang.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build -j8      # run the test suite
```

## Using the compiler

```sh
nirea run Hello.ni                    # build and run a single file
nirea build Hello.ni -o hello         # build an executable
nirea build path/to/project           # build a project into target/
nirea check path/to/project           # check without building
```

A project keeps its sources in `src/main/nirea`. `nirea build` writes the generated C to `target/generated-c/` and the executable to `target/nirea-app`. When you build a single file, the compiler also loads the files it imports.

Options:

- `-o FILE` sets the executable path.
- `--emit-c DIR` keeps the generated C in `DIR`.
- `--cc COMPILER` selects the C compiler; the default is `$CC`, then `cc`.
- `-lLIB` and `-LDIR` are passed to the C compiler, for example to link C libraries used through `extern "C"`.

## Documentation

- [Language design specification](docs/language-design.md) (v0.8, preliminary)
- [AGENTS.md](AGENTS.md): guidance for AI coding agents and contributors
