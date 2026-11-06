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
- No hidden values: objects are created explicitly, as in `Circle circle = Circle();`, and a local variable must be assigned before it is used.
- Ahead-of-time compilation to native code, with direct interoperability with C libraries.
- A small language core, with further functionality in optional libraries.

## Quick start

You need CMake 3.16 or later and GCC or Clang.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build -j8      # run the test suite
build/nirea run Hello.ni        # build and run a program
```

On Windows, see [Windows](docs/guide/windows.md).

## Documentation

- [The Nirea guide](docs/guide/README.md): the language by example, one topic per page.
  - [Getting started](docs/guide/getting-started.md)
  - [Basics](docs/guide/basics.md)
  - [Strings](docs/guide/strings.md)
  - [Arrays](docs/guide/arrays.md)
  - [Structs and interfaces](docs/guide/structs.md)
  - [Enums](docs/guide/enums.md)
  - [Ownership](docs/guide/ownership.md)
  - [References](docs/guide/references.md)
  - [Global objects](docs/guide/global-objects.md)
  - [Namespaces and projects](docs/guide/projects.md)
  - [Calling C](docs/guide/c-interop.md)
  - [Windows](docs/guide/windows.md)
- [Language specification](docs/spec/README.md) (v0.14, preliminary): the exact rules, one chapter per page.
- [AGENTS.md](AGENTS.md): guidance for AI coding agents and contributors.

## Status

Nirea is at an early stage. The first compiler, `nirea`, implements version 0.14 of the specification. It translates Nirea to C and builds native executables with GCC or Clang.
