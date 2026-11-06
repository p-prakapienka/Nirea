# 6. Types

## 6.1 Primitive types

Nirea provides a deliberately small set of built-in primitive types.

| Type      | Width           | Description              |
| --------- | --------------- | ------------------------ |
| `boolean` | Logical boolean | True / false             |
| `char`    | 8 bits          | Byte-sized character     |
| `int8`    | 8 bits          | Signed integer           |
| `int16`   | 16 bits         | Signed integer           |
| `int`     | 32 bits         | Default signed integer   |
| `int64`   | 64 bits         | Signed integer           |
| `uint8`   | 8 bits          | Unsigned integer         |
| `uint16`  | 16 bits         | Unsigned integer         |
| `uint`    | 32 bits         | Default unsigned integer |
| `uint64`  | 64 bits         | Unsigned integer         |
| `float`   | 32 bits         | IEEE 754 binary32        |
| `float64` | 64 bits         | IEEE 754 binary64        |

`char` is an 8-bit type holding values 0 to 255. Unicode and UTF-8 string handling belong to library functionality rather than the primitive type system. `char` supports comparison and explicit casts to and from integer types, but no arithmetic.

## 6.2 Conversion rules

- No implicit signed-to-unsigned conversions.
- No implicit narrowing conversions.
- Explicit casts are required for potentially lossy conversions.
- `int` is always 32 bits.
- `uint` is always 32 bits.
- `float` is always binary32.
- `float64` is always binary64.

A conversion is implicit only when every value of the source type is exactly representable in the target type:

| From     | Converts implicitly to                                                  |
| -------- | ----------------------------------------------------------------------- |
| `int8`   | `int16`, `int`, `int64`, `float`, `float64`                             |
| `int16`  | `int`, `int64`, `float`, `float64`                                      |
| `int`    | `int64`, `float64`                                                      |
| `uint8`  | `uint16`, `uint`, `uint64`, `int16`, `int`, `int64`, `float`, `float64` |
| `uint16` | `uint`, `uint64`, `int`, `int64`, `float`, `float64`                    |
| `uint`   | `uint64`, `int64`, `float64`                                            |
| `float`  | `float64`                                                               |

`int64`, `uint64` and `float64` convert to nothing else implicitly. `boolean`, `char`, enums and `String` never convert implicitly.

Other numeric conversions need an explicit cast, written as in Java: `(int) value`. Casts are allowed between any two numeric types, and between `char` and integer types.

- Integer-to-integer and integer-to-`char` casts keep the low bits of the two's-complement representation, as in Java and C. For example, `(uint8) 300` is `44` and `(char) 321` is `'A'`. A `char` cast to an integer type gives its value, 0 to 255.
- Float-to-integer casts truncate toward zero. Casting NaN or an infinity is a runtime error ([§6.4](#64-arithmetic-and-runtime-errors)), and so is a value whose truncation doesn't fit the target type: `(uint8) -0.5` is `0`, but `(uint8) -1.0` is an error. Floating-point values can't be cast to `char`.
- Conversions to a floating-point type round to the nearest representable value.

## 6.3 Literals

- Integer literals: decimal (`42`), hexadecimal (`0x2A` or `0x2a`) and binary (`0b101010`). Underscores can appear only between digits (`1_000_000`). A decimal literal other than `0` can't start with `0`. The prefixes can also be written `0X` and `0B`. Hexadecimal and binary literals denote values, not bit patterns: `0x80000000` is 2147483648, which doesn't fit `int`.
- Floating-point literals have a fractional part, an exponent (`e` or `E`) or both: `1.5`, `2.0e-3`, `1E9`. A literal needs a digit on both sides of the decimal point.
- Character literals: `'a'`, `'\n'`.
- String literals: `"text"`.
- Boolean literals: `true` and `false`.
- Escape sequences in character and string literals: `\n`, `\t`, `\r`, `\0`, `\\`, `\'`, `\"` and `\xHH`. Any other escape is an error. A character literal holds exactly one byte.

Numeric literals take their type from context. An integer literal can be used as any integer or floating-point type that represents its value exactly. A floating-point literal can be used as `float` or `float64`; it is rounded to the nearest `float` when used as `float`. A literal that doesn't fit its target type is a compile-time error. With no context, an integer literal is an `int` and a floating-point literal is a `float64`.

A negated literal counts as a single literal, so `int8 low = -128;` is valid. The context of an operand is the other operand, or for an operation on two literals, the context of the whole expression: in `float x = 2 * 1.5;` both literals are `float`. Division and remainder between two integer literals are always integer operations, as in C and Java: `float64 half = 1 / 2;` gives `0.0`, and `float x = 1 / 2;` is an error because `int` doesn't convert to `float` implicitly.

## 6.4 Arithmetic and runtime errors

Arithmetic and comparison operators need operands of the same type after implicit conversion ([§6.2](#62-conversion-rules)): one operand is converted to the other's type, and arithmetic produces a value of that type. If neither operand converts to the other's type, a cast is needed. For example, `int8 + int` is an `int`, but `uint8 + int8` and `int + float` are errors. Unary minus isn't allowed on unsigned types. `+` also concatenates strings ([§6.5](#65-strings)).

Integer division truncates toward zero, and the remainder has the sign of the dividend, as in C and Java: `-7 / 2` is `-3` and `-7 % 2` is `-1`.

Integer overflow is a runtime error. So are:

- integer division or remainder by zero;
- dividing the smallest value of a signed type by `-1`, which overflows (the remainder of that division is `0`, not an error);
- shifting by a negative amount, or by at least the width of the shifted type;
- an array or string index out of range ([§6.5](#65-strings), [§6.6](#66-arrays));
- creating an array with a negative length, or more than 2147483647 elements ([§6.6](#66-arrays));
- an invalid float-to-integer cast ([§6.2](#62-conversion-rules));
- text that `toInt`, `toInt64` or `toFloat64` can't parse ([§6.5](#65-strings));
- running out of memory.

A runtime error stops the program immediately, with a message naming the error and its source location and a non-zero exit status. It can't be caught; recoverable error handling is an open question ([§18](18-open-questions.md)).

`<<` shifts in zeros and discards the bits shifted out; it isn't checked for overflow. `>>` is an arithmetic shift for signed types and a logical shift for unsigned types. The shift amount can have any integer type. `&`, `|`, `^` and `~` work on integer types.

Floating-point arithmetic follows IEEE 754: division or remainder by zero produces an infinity or NaN and isn't an error. `%` on floating-point values gives a remainder with the sign of the dividend, like C's `fmod`.

## 6.5 Strings

`String` is a built-in object type holding a sequence of bytes. A `String` follows exactly the same rules as a struct ([§7](07-ownership.md)): assigning it or returning it transfers ownership, passing it to a value parameter passes a copy ([§7.3](07-ownership.md#73-passing-values-and-references)), a `String*` parameter refers to it in place, and `copy()` creates an independent copy. A `String` field or array element is part of its owner. Each `String` has its own bytes, freed automatically when its owner ends ([§7.6](07-ownership.md#76-resource-lifecycle)).

The only difference from a struct is the string literal: `"text"` creates a new `String` object each time it is evaluated, so it can initialise or be assigned to any `String` variable. A `String` can be changed through its methods; changes never affect other `String` objects.

```nirea
String greeting = "Hello";
greeting.append(", Nirea");
greeting.appendChar('!');
println(greeting);                 // Hello, Nirea!

String copy = greeting.copy();    // both stay usable
copy.setCharAt(0, 'J');
println(greeting);                 // Hello, Nirea!
println(copy);                     // Jello, Nirea!

String moved = greeting;           // greeting is no longer accessible
```

Built-in methods:

| Method                               | Result                                                                   |
| ------------------------------------ | ------------------------------------------------------------------------ |
| `int length()`                       | The number of bytes                                                      |
| `char charAt(int index)`             | The byte at `index`                                                      |
| `void setCharAt(int index, char c)`  | Replaces the byte at `index`                                             |
| `void append(String text)`           | Adds the bytes of `text` at the end                                      |
| `void appendChar(char c)`            | Adds one byte at the end                                                 |
| `String substring(int from, int to)` | A new `String` with the bytes from `from` up to, but not including, `to` |
| `int indexOf(String text)`           | The first index where `text` occurs, or `-1`; `0` for an empty `text`    |
| `int toInt()`                        | The text parsed as an `int`                                              |
| `int64 toInt64()`                    | The text parsed as an `int64`                                            |
| `float64 toFloat64()`                | The text parsed as a `float64`                                           |
| `String copy()`                      | An independent copy ([§7.2](07-ownership.md#72-explicit-copying))                                               |
| `boolean equals(String text)`        | Whether both strings hold the same bytes ([§6.9](#69-equality))                          |

The `String` arguments of `append`, `indexOf` and `equals` are value parameters ([§7.3](07-ownership.md#73-passing-values-and-references)), so a string can be passed to its own methods: `s.append(s)` doubles `s`, and `s.indexOf(s)` is `0`.

`String.of(value)` creates a new `String` from a value of a primitive or enum type, with the same text `print` would write ([§15](15-libraries.md)): `String.of(42)` is `"42"`.

An index outside the string (for `charAt` and `setCharAt`, outside `0` to `length() - 1`; for `substring`, unless `0 <= from <= to <= length()`) is a runtime error, as for arrays ([§6.4](#64-arithmetic-and-runtime-errors)). `toInt`, `toInt64` and `toFloat64` accept an optional `-` or `+` followed by a decimal number, with a fractional part and exponent only for `toFloat64` (which also accepts `NaN`, `Infinity` and `-Infinity`). Anything else, including a value out of range for the type, is a runtime error.

`a + b` creates a new `String` holding the bytes of `a` followed by the bytes of `b`. Both operands must be strings; other values are converted explicitly with `String.of`, as in `"count: " + String.of(count)`. The operands are used in place, like `String*` parameters: they don't change and don't transfer ownership, and either can be a reference. `a += b` appends `b` to `a`, with the same result as `a = a + b`; like any assignment it needs a variable, field or array element on the left, so through a `String*` reference use `append`.

```nirea
String name = "Nirea";
String greeting = "Hello, " + name + "!";
greeting += " v" + String.of(8);
println(greeting);                 // Hello, Nirea! v8
```

Strings can be compared with `==` ([§6.9](#69-equality)), printed ([§15](15-libraries.md)) and passed to C functions ([§14](14-c-interop.md)).

## 6.6 Arrays

An array type `T[]` holds a sequence of elements of type `T`. As in Java, the type doesn't include a size: an array's length is fixed when the array is created, and never changes.

```nirea
int[] values = int[4];                             // 4 elements, all 0
int[] primes = {2, 3, 5};
Vec2[] corners = {Vec2(0.0, 0.0), Vec2(1.0, 1.0)};
int[][] grid = int[2][3];                          // 2 rows of 3 ints, all 0

values[0] = 10;
int count = values.length;                         // 4
grid[1][2] = 7;
int size = count * 2;
float[] samples = float[size];                     // the length can be computed
```

- `T[n]` creates an array of `n` elements, each holding its zero value ([§6.8](#68-zero-values-and-initialisation)), even when `T` is a struct that declares a constructor. `n` can be any integer expression. A negative `n`, or one above 2147483647, is a runtime error ([§6.4](#64-arithmetic-and-runtime-errors)).
- `T[a][b]` creates an array of `a` arrays, each of `b` elements. All sizes are evaluated, left to right, before any array is created. The rows of a `T[][]` are separate arrays and can be replaced, so they can have different lengths.
- An array initialiser `{...}` creates an array with one element per value. It can be used only to initialise a variable declaration.
- `a[i]` accesses an element. The index can have any integer type. An index outside `0` to `a.length - 1` is a runtime error.
- `a.length` is the number of elements, as an `int`.
- Arrays are owned values, like structs: assignment transfers ownership of them and `copy()` copies them ([§7](07-ownership.md)). An array's elements are stored outside its owner, and freed with the array ([§7.6](07-ownership.md#76-resource-lifecycle)).
- Arrays can be local variables, fields, elements of other arrays, parameters (by value or by reference, [§7.3](07-ownership.md#73-passing-values-and-references)) and return values. A reference to an array is written `int[]* values`.
- Growable arrays, such as lists, are an open question ([§18](18-open-questions.md)).

## 6.7 Enums

Enums are simple named values. Variants with associated data are an open question ([§18](18-open-questions.md)).

```nirea
enum State {
    IDLE,
    RUNNING,
    STOPPED
}
```

Enum values are written qualified with the enum name, such as `State.IDLE`, except in the `case` labels of a switch on that enum ([§10.1](10-control-flow.md#101-switch-expressions)). Enums support `==` and `!=`. They aren't owned, so they're copied like primitive values. Printing an enum value prints its name.

## 6.8 Zero values and initialisation

Every type has a zero value: `0` for numbers, `false` for `boolean`, `'\0'` for `char`, `""` for `String`, the first value for an enum, all fields zero for a struct and an empty array for an array type.

Fields and array elements start at their zero value: a struct's fields when it is created with `T()` or before its constructor body runs ([§5.4](05-syntax.md#54-constructors)), and an array's elements when it is created with `T[n]` ([§6.6](#66-arrays)). The zero value of an array type is an empty array, with length `0`.

Local variables have no zero value. A local variable declared without an initialiser, such as `Circle circle;`, is only declared: it has no value until one is assigned to it as a whole, and using it before then is a compile error. Declaration and assignment can be split:

```nirea
Circle circle;                 // declared, no value yet
circle.radius = 2.0;           // error: circle has no value yet
circle = Circle();
circle.radius = 2.0;           // ok
```

This applies to every type, including numbers: `int count; count++;` is an error. Reading a variable, assigning to its fields or elements, calling a method on it, passing it to a function, `++`, `--` and compound assignments such as `+=` all use it. As with ownership transfers ([§7.1](07-ownership.md#71-ownership-transfer)), the compiler follows assignments through all control flow: a variable must have been assigned on every path that reaches a use. No value can be read uninitialised.

## 6.9 Equality

`==` and `!=` compare values by their structure:

- Numbers, `char`, `boolean` and enum values compare their values. Floating-point comparison follows IEEE 754, so NaN is not equal to itself.
- Strings compare their bytes.
- Structs compare their fields, in declaration order, with `==`.
- Arrays compare their elements, in order, with `==`.

Both operands must be values of the same type, after implicit conversion for numbers ([§6.2](#62-conversion-rules)).

A reference ([§9](09-references.md)) isn't a value, so using one as an operand of `==` or `!=` is a compile error, whether the other operand is a value or another reference. Readers coming from different languages would expect `==` on references to compare either addresses or values, so Nirea makes the comparison explicit: `a.equals(b)` compares the values `a` and `b` refer to, by structure, exactly as `==` compares two values. Nirea doesn't compare addresses.

`equals` is built in for every struct, array and `String` type. The object it is called on is used in place, like `this`, and its argument is a value parameter ([§7.3](07-ownership.md#73-passing-values-and-references)), so `a.equals(a)` is allowed. Nothing changes owner. Either side can be a reference or a value, so `answer.equals("yes")` compares a `String*` parameter with a literal. Fields reached through a reference are values and can be compared with `==` directly (`a.x == b.x`). Interface values have no `equals`.

```nirea
boolean sameContents(Engine* a, Engine* b) {
    return a.equals(b);          // compares the engines' fields
}
```

---

[← 5. Basic syntax](05-syntax.md) · [Contents](README.md) · [7. Ownership and memory safety →](07-ownership.md)
