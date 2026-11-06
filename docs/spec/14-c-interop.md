# 14. C interoperability

Nirea is designed to integrate with existing C libraries and functions.

It should be possible to declare and call external C functions directly.

An `extern "C"` block declares C functions that Nirea code can call. It can appear at the top level of a file or inside a namespace.

```nirea
extern "C" {
    int printf(String* format, ...);
    float64 sqrt(float64 x);
}

void main() {
    printf("Hello from Nirea!\n");
    printf("%d\n", 42);
}
```

Parameter types can be primitive types and `String*`. A `String*` parameter gives the C function a pointer to the string's bytes, followed by a null byte, for the duration of the call; the string stays with its owner. Return types can be primitive types or `void`. A final `...` declares a variadic function; variadic arguments must be primitives or strings, and strings are passed like `String*`. Types map to C as follows:

| Nirea                               | C                                             |
| ----------------------------------- | --------------------------------------------- |
| `boolean`                           | `bool`                                        |
| `char`                              | `char`                                        |
| `int8`, `int16`, `int`, `int64`     | `int8_t`, `int16_t`, `int32_t`, `int64_t`     |
| `uint8`, `uint16`, `uint`, `uint64` | `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t` |
| `float`, `float64`                  | `float`, `double`                             |
| `String*`                           | `const char *`, null-terminated               |
| `void`                              | `void`                                        |

A C function can't be named `main` or a C keyword, and names starting with `nr_`, or with `nr` or `nri` followed by a digit, are reserved for the compiler's generated code.

The compiler can't check that a declaration matches the real C function. A wrong declaration has undefined behaviour, which is why the C boundary is an explicit safety boundary.

Struct, array, pointer and callback types at the C boundary remain to be specified ([§18](18-open-questions.md)).

## Interoperability goals

- Call existing C functions.
- Link against C libraries.
- Use C-compatible structs.
- Support native ABI conventions.
- Expose Nirea functions to C where practical.
- Support system libraries and embedded development.

C interoperability is an explicit safety boundary.

Raw pointers may exist in generated C and the FFI implementation, but they are not part of ordinary Nirea source syntax.

The compiler must ensure that interactions with C do not silently invalidate Nirea's ownership guarantees.

---

[← 13. Native compilation](13-compilation.md) · [Contents](README.md) · [15. Libraries →](15-libraries.md)
