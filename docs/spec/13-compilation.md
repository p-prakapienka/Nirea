# 13. Native compilation

Nirea is designed for ahead-of-time native compilation.

There is no mandatory JVM, garbage collector or managed runtime.

## 13.1 Initial compiler architecture

The first compiler implementation will be a source-to-source translator.

```text
Nirea source files
       |
       v
Nirea compiler
       |
       v
Lexer and parser
       |
       v
AST construction
       |
       v
Name and type resolution
       |
       v
Ownership and lifetime analysis
       |
       v
C code generation
       |
       v
Generated C source
       |
       v
GCC / Clang
       |
       v
Native executable
```

## 13.2 Implementation limits

The first compiler rejects programs beyond these limits:

- Expressions nested more than 1000 levels deep, counting each operator, call, member access and parenthesis.
- Statements and array initialisers nested more than 1000 levels deep.

## 13.3 Why C as an intermediate language?

This approach offers several advantages:

- No initial machine-code backend implementation.
- Reuse of mature compiler optimisations.
- Access to established platform toolchains.
- Simpler cross-platform development.
- Native linking.
- Natural C interoperability.

Generated C is an implementation detail, not a language programmers are expected to maintain.

Nirea's compiler is responsible for enforcing Nirea's type, ownership and safety rules before generating C.

The architecture should allow a future LLVM or native backend without changing language semantics.

---

[← 12. Source files and imports](12-source-files.md) · [Contents](README.md) · [14. C interoperability →](14-c-interop.md)
