# 15. Libraries

Nirea deliberately keeps its language core small.

Additional functionality is provided through optional libraries.

Potential areas:

- Collections and containers.
- Strings and text processing.
- File system access.
- Networking.
- Mathematics.
- Concurrency.
- Synchronisation.
- Graphics.
- Audio.
- Hardware and embedded development.

The compiler and runtime should remain as small as reasonably possible.

Until a standard library exists, the compiler provides two built-in functions for output: `print(value)` and `println(value)`. They accept one value of a primitive, enum or `String` type, and `println()` with no argument prints an empty line. A `String` argument is used in place, like a `String*` parameter. Values print as follows:

- Integers print in decimal, and `boolean` values as `true` or `false`.
- A `char` prints as its byte.
- A string prints its bytes.
- An enum value prints the name of the value, without the enum or namespace name.
- Floating-point values print in their shortest form that reads back as the same value of the same type, always with a decimal point or exponent. Magnitudes from `0.001` up to `10000000` use plain notation (`1.0`, `0.1`, `123.25`); others use an exponent (`1.0E10`, `1.5E-5`). NaN prints as `NaN`, and infinities as `Infinity` and `-Infinity`.

`print` and `println` write to the C library's standard output, the same stream C functions such as `printf` use, so output appears in program order.

---

[← 14. C interoperability](14-c-interop.md) · [Contents](README.md) · [16. Project management and build structure →](16-build.md)
