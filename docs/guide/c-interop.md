# Calling C

Nirea code can call C functions directly. Declare them in an `extern "C"` block, at the top level of a file or in a namespace:

```nirea
extern "C" {
    int printf(String* format, ...);
    float64 sqrt(float64 x);
}

void main() {
    printf("Hello from C, %d\n", 42);        // Hello from C, 42
    println(sqrt(6.25));                     // 2.5
}
```

- Parameters can have primitive types or `String*`. A `String*` gives the C function the string's bytes, followed by a null byte, for the length of the call.
- Return types can be primitive types or `void`.
- A final `...` declares a variadic function, such as `printf`. Its extra arguments must be primitives or strings.
- `print`, `println` and C's `printf` write to the same output, so the output appears in program order.

The types map to C as follows:

| Nirea                               | C                                             |
| ----------------------------------- | --------------------------------------------- |
| `boolean`                           | `bool`                                        |
| `char`                              | `char`                                        |
| `int8`, `int16`, `int`, `int64`     | `int8_t`, `int16_t`, `int32_t`, `int64_t`     |
| `uint8`, `uint16`, `uint`, `uint64` | `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t` |
| `float`, `float64`                  | `float`, `double`                             |
| `String*`                           | `const char *`, null-terminated               |

To link a C library, pass it to the compiler: `nirea build App.ni -lcurl -L/opt/curl/lib`. The C math library is linked by default.

The compiler can't check that a declaration matches the real C function, and a wrong one can crash the program. This is the one place where Nirea's safety depends on you. Structs, arrays, pointers and callbacks can't cross into C yet.

---

[← Namespaces and projects](projects.md) · [Contents](README.md) · [Windows →](windows.md)
