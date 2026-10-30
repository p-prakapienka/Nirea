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

Nirea is in the language-definition phase. There is no compiler yet; the first one is planned to translate Nirea to C and build native executables with GCC or Clang.

## Documentation

- [Language design specification](docs/language-design.md) (v0.2, preliminary)
- [AGENTS.md](AGENTS.md): guidance for AI coding agents and contributors
