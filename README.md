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
- Values and references: a variable without `*` holds an object of its own, so assignment copies; a reference (`T*`) works on an existing object in place.
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

On Windows, see [Windows](docs/guide/12-windows.md).

## Documentation

- [The Nirea guide](docs/guide/README.md): the language by example, one topic per page.
  - [1. Getting started](docs/guide/01-getting-started.md)
  - [2. Basics](docs/guide/02-basics.md)
  - [3. Strings](docs/guide/03-strings.md)
  - [4. Arrays](docs/guide/04-arrays.md)
  - [5. Structs and interfaces](docs/guide/05-structs.md)
  - [6. Enums](docs/guide/06-enums.md)
  - [7. Ownership](docs/guide/07-ownership.md)
  - [8. References](docs/guide/08-references.md)
  - [9. Global objects](docs/guide/09-global-objects.md)
  - [10. Namespaces and projects](docs/guide/10-projects.md)
  - [11. Calling C](docs/guide/11-c-interop.md)
  - [12. Windows](docs/guide/12-windows.md)
- [Language specification](docs/spec/README.md) (v0.15, preliminary): the exact rules, one chapter per page.
- [AGENTS.md](AGENTS.md): guidance for AI coding agents and contributors.

## Status

Nirea is at an early stage. The first compiler, `nirea`, implements version 0.15 of the specification. It translates Nirea to C and builds native executables with GCC or Clang.
