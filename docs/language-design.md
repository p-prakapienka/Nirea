# Nirea — Language Design Specification

**Version:** 0.9 — Value parameters receive copies
**Status:** Preliminary Specification

Version 0.3 resolved the open questions the first compiler depends on: constructors, strings, arrays, integer overflow, parameter passing and ownership transfer, interface dispatch, constants, imports and the C function interface. Version 0.4 adds value and reference parameters (§7.3): `Engine engine` receives its own object and becomes its owner, `Engine* engine` refers to the caller's object in place. Version 0.5 makes `String` a mutable object type with built-in methods (§6.5), frees owned objects automatically when their owner ends (§7.6), and defines `==` as structural equality (§6.9). Version 0.6 separates comparing values (`==`) from comparing references (`is`, §6.10). Version 0.7 adds `equals` to compare the values references point to (§6.9). Version 0.8 adds string concatenation with `+` (§6.5). Version 0.9 makes value parameters receive copies of their arguments (§7.3). Section 18 lists what remains open.

## 1. Vision

Nirea is a statically typed, memory-safe, native programming language designed for systems programming and general-purpose development.

Its syntax takes inspiration from Java, while its programming model is closer to C: free functions, structs, explicit operations and native execution.

Nirea aims to combine:

- The readability and familiarity of Java.
- The simplicity and directness of C.
- Memory safety through exclusive ownership.
- Native compilation and interoperability with existing C libraries.
- A deliberately small language core.
- Optional libraries rather than mandatory runtime functionality.

Nirea is not intended to be another object-oriented language. Classes, inheritance and complex runtime abstractions are deliberately excluded.

## 2. Core principles

1. Explicit code is preferable to implicit behaviour.
2. Simplicity is preferable to syntactic convenience.
3. Memory safety should be enforced by the compiler.
4. Native execution is the default.
5. The language core should remain small.
6. Functionality should be provided by optional libraries wherever possible.
7. Existing C libraries and ecosystems should remain accessible.
8. No hidden operations or surprising language behaviour.

## 3. Language characteristics

| Characteristic           | Design                             |
| ------------------------ | ---------------------------------- |
| Typing                   | Static                             |
| Syntax                   | Java-inspired                      |
| Programming model        | C-inspired                         |
| Compilation              | Ahead-of-time                      |
| Initial compiler backend | C transpilation                    |
| Native executable        | GCC / Clang or compatible compiler |
| Memory management        | Exclusive ownership                |
| Raw pointers             | Not exposed in Nirea               |
| Data abstraction         | Structs with methods               |
| Interfaces               | Supported                          |
| Classes                  | Not supported                      |
| Inheritance              | Not supported                      |
| Runtime                  | No mandatory managed runtime       |
| C interoperability       | Native ABI interoperability        |
| Libraries                | Optional                           |

## 4. Naming conventions

Nirea follows familiar Java naming conventions.

| Element   | Convention       | Example               |
| --------- | ---------------- | --------------------- |
| Namespace | PascalCase       | `Audio`               |
| Struct    | PascalCase       | `AudioEngine`         |
| Interface | PascalCase       | `Drawable`            |
| Enum      | PascalCase       | `AudioState`          |
| Function  | camelCase        | `calculateDistance()` |
| Variable  | camelCase        | `sampleRate`          |
| Constant  | UPPER_SNAKE_CASE | `MAX_BUFFER_SIZE`     |

## 5. Basic syntax

Nirea uses familiar Java/C-style syntax.

- Curly braces define blocks.
- Semicolons terminate statements.
- Functions are declared independently of structs.
- Types are explicitly declared.
- Control-flow syntax remains familiar.

Example:

```nirea
void main() {
    println("Hello, Nirea!");
}
```

### 5.1 Functions

Functions do not require a containing class or struct.

```nirea
int add(int a, int b) {
    return a + b;
}

void main() {
    int result = add(10, 20);
    println(result);
}
```

Functions, methods and constructors can't be overloaded: each name is declared once in its scope (§18). Declarations can be used regardless of their order in the file: functions can call functions declared later, and recursion is allowed. The same holds for structs, interfaces, enums and constants.

A program has exactly one top-level `main` function, declared as `void main()` or `int main()`. The value returned by `int main()` is the process exit status.

### 5.2 Structs

Nirea has one primary data abstraction: `struct`.

Structs contain fields and methods.

There is no distinction between classes and structs.

```nirea
struct Vec2 {
    float x;
    float y;

    float lengthSquared() {
        return x * x + y * y;
    }

    void scale(float factor) {
        x = x * factor;
        y = y * factor;
    }
}
```

Structs are intended to support predictable native data layouts and low-level development.

Fields are stored inline, in declaration order. A struct can't contain itself, directly or through other structs or arrays.

Inside a method, fields and other methods of the same struct can be used by name, or through `this`: `this.x` and `x` are the same field. `this` is a non-owning reference to the value the method was called on (§7.3).

### 5.3 Interfaces

Interfaces define behavioural contracts.

Structs can implement one or multiple interfaces using `:`.

```nirea
interface Drawable {
    void draw();
}

struct Circle : Drawable {
    float radius;

    void draw() {
        println("Drawing circle");
    }
}
```

Multiple interfaces:

```nirea
struct Circle : Drawable, Serializable {
    // ...
}
```

Design rules:

- Interfaces have no instance state.
- Interfaces define method contracts.
- Structs implement interfaces.
- Struct inheritance is not supported.
- Abstract classes are not supported.
- Multiple interface implementation is supported.

A struct implements an interface only by listing it after `:`. It must then declare a public method with the same name, parameter types and return type for every method of the interface. Having matching methods without listing the interface doesn't implement it.

Interface types can be used only as reference parameter types, written `Drawable*` (§7.3). An interface parameter is a non-owning reference to a struct value that implements the interface, and calls through it use dynamic dispatch. Any struct that implements the interface can be passed for it, and an interface-typed parameter can itself be passed on to another parameter of the same interface type.

```nirea
void render(Drawable* shape) {
    shape.draw();
}

void main() {
    Circle circle;
    circle.radius = 2.0;
    render(circle);
}
```

Variables, fields, array elements and return values of interface type are not supported yet (§18).

### 5.4 Constructors

A struct can declare one constructor: a member named after the struct, with a parameter list and no return type. A constructor is called like a function, using the struct's name, and produces a new value. There is no `new` keyword: structs are values, not heap objects.

```nirea
struct Vec2 {
    float x;
    float y;

    Vec2(float x, float y) {
        this.x = x;
        this.y = y;
    }
}

void main() {
    Vec2 position = Vec2(1.5, 2.0);
}
```

All fields hold their zero value (§6.8) before the constructor body runs, so no field is ever read uninitialised. A constructor can set private fields.

A struct that declares no constructor can be created with `T()`, which produces a zero-initialised value. A declaration without an initialiser, such as `Vec2 v;`, is equivalent. For a struct that declares a constructor, `T(...)` always calls that constructor, and a declaration without an initialiser is an error. The exception is storage owned by another value: fields and array elements of that struct type start at their zero value (§6.8).

### 5.5 Constants

Constants are declared with `const` at the top level of a file or inside a namespace.

```nirea
const int MAX_BUFFER_SIZE = 1024;
const String APP_NAME = "Nirea";
```

A constant has a primitive, enum or `String` type. Its initialiser is a literal, a negated numeric literal or an enum value. A `String` constant is a named literal: each use creates a new `String` object, like writing the literal (§6.5).

There are no global variables. Mutable state lives in local variables and in the values they own.

## 6. Types

### 6.1 Primitive types

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

### 6.2 Conversion rules

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
- Float-to-integer casts truncate toward zero. Casting NaN or an infinity is a runtime error (§6.4), and so is a value whose truncation doesn't fit the target type: `(uint8) -0.5` is `0`, but `(uint8) -1.0` is an error. Floating-point values can't be cast to `char`.
- Conversions to a floating-point type round to the nearest representable value.

### 6.3 Literals

- Integer literals: decimal (`42`), hexadecimal (`0x2A` or `0x2a`) and binary (`0b101010`). Underscores can appear only between digits (`1_000_000`). A decimal literal other than `0` can't start with `0`. The prefixes can also be written `0X` and `0B`. Hexadecimal and binary literals denote values, not bit patterns: `0x80000000` is 2147483648, which doesn't fit `int`.
- Floating-point literals have a fractional part, an exponent (`e` or `E`) or both: `1.5`, `2.0e-3`, `1E9`. A literal needs a digit on both sides of the decimal point.
- Character literals: `'a'`, `'\n'`.
- String literals: `"text"`.
- Boolean literals: `true` and `false`.
- Escape sequences in character and string literals: `\n`, `\t`, `\r`, `\0`, `\\`, `\'`, `\"` and `\xHH`. Any other escape is an error. A character literal holds exactly one byte.

Numeric literals take their type from context. An integer literal can be used as any integer or floating-point type that represents its value exactly. A floating-point literal can be used as `float` or `float64`; it is rounded to the nearest `float` when used as `float`. A literal that doesn't fit its target type is a compile-time error. With no context, an integer literal is an `int` and a floating-point literal is a `float64`.

A negated literal counts as a single literal, so `int8 low = -128;` is valid. The context of an operand is the other operand, or for an operation on two literals, the context of the whole expression: in `float x = 2 * 1.5;` both literals are `float`. Division and remainder between two integer literals are always integer operations, as in C and Java: `float64 half = 1 / 2;` gives `0.0`, and `float x = 1 / 2;` is an error because `int` doesn't convert to `float` implicitly.

### 6.4 Arithmetic and runtime errors

Arithmetic and comparison operators need operands of the same type after implicit conversion (§6.2): one operand is converted to the other's type, and arithmetic produces a value of that type. If neither operand converts to the other's type, a cast is needed. For example, `int8 + int` is an `int`, but `uint8 + int8` and `int + float` are errors. Unary minus isn't allowed on unsigned types. `+` also concatenates strings (§6.5).

Integer division truncates toward zero, and the remainder has the sign of the dividend, as in C and Java: `-7 / 2` is `-3` and `-7 % 2` is `-1`.

Integer overflow is a runtime error. So are:

- integer division or remainder by zero;
- dividing the smallest value of a signed type by `-1`, which overflows (the remainder of that division is `0`, not an error);
- shifting by a negative amount, or by at least the width of the shifted type;
- an array or string index out of range (§6.5, §6.6);
- an invalid float-to-integer cast (§6.2);
- text that `toInt`, `toInt64` or `toFloat64` can't parse (§6.5);
- running out of memory.

A runtime error stops the program immediately, with a message naming the error and its source location and a non-zero exit status. It can't be caught; recoverable error handling is an open question (§18).

`<<` shifts in zeros and discards the bits shifted out; it isn't checked for overflow. `>>` is an arithmetic shift for signed types and a logical shift for unsigned types. The shift amount can have any integer type. `&`, `|`, `^` and `~` work on integer types.

Floating-point arithmetic follows IEEE 754: division or remainder by zero produces an infinity or NaN and isn't an error. `%` on floating-point values gives a remainder with the sign of the dividend, like C's `fmod`.

### 6.5 Strings

`String` is a built-in object type holding a sequence of bytes. A `String` follows exactly the same rules as a struct (§7): assigning it or returning it transfers ownership, passing it to a value parameter passes a copy (§7.3), a `String*` parameter refers to it in place, and `copy()` creates an independent copy. A `String` field or array element is part of its owner. Each `String` has its own bytes, freed automatically when its owner ends (§7.6).

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
| `void append(String* text)`          | Adds the bytes of `text` at the end                                      |
| `void appendChar(char c)`            | Adds one byte at the end                                                 |
| `String substring(int from, int to)` | A new `String` with the bytes from `from` up to, but not including, `to` |
| `int indexOf(String* text)`          | The first index where `text` occurs, or `-1`; `0` for an empty `text`    |
| `int toInt()`                        | The text parsed as an `int`                                              |
| `int64 toInt64()`                    | The text parsed as an `int64`                                            |
| `float64 toFloat64()`                | The text parsed as a `float64`                                           |
| `String copy()`                      | An independent copy (§7.2)                                               |
| `boolean equals(String* text)`       | Whether both strings hold the same bytes (§6.9)                          |

`String.of(value)` creates a new `String` from a value of a primitive or enum type, with the same text `print` would write (§15): `String.of(42)` is `"42"`.

An index outside the string (for `charAt` and `setCharAt`, outside `0` to `length() - 1`; for `substring`, unless `0 <= from <= to <= length()`) is a runtime error, as for arrays (§6.4). `toInt`, `toInt64` and `toFloat64` accept an optional `-` or `+` followed by a decimal number, with a fractional part and exponent only for `toFloat64` (which also accepts `NaN`, `Infinity` and `-Infinity`). Anything else, including a value out of range for the type, is a runtime error.

`a + b` creates a new `String` holding the bytes of `a` followed by the bytes of `b`. Both operands must be strings; other values are converted explicitly with `String.of`, as in `"count: " + String.of(count)`. The operands are used in place, like `String*` parameters: they don't change and don't transfer ownership, and either can be a reference. `a += b` appends `b` to `a`, with the same result as `a = a + b`; like any assignment it needs a variable, field or array element on the left, so through a `String*` reference use `append`.

```nirea
String name = "Nirea";
String greeting = "Hello, " + name + "!";
greeting += " v" + String.of(8);
println(greeting);                 // Hello, Nirea! v8
```

Strings can be compared with `==` (§6.9), printed (§15) and passed to C functions (§14).

### 6.6 Arrays

An array type `T[N]` holds exactly `N` elements of type `T`, stored inline. `N` is an integer literal or integer constant greater than zero.

```nirea
int[4] values;                                     // all elements are 0
int[3] primes = {2, 3, 5};
Vec2[2] corners = {Vec2(0.0, 0.0), Vec2(1.0, 1.0)};
int[2][3] grid;                                    // 2 rows of 3 ints

values[0] = 10;
int count = values.length;                         // 4
grid[1][2] = 7;
```

- An array initialiser `{...}` lists exactly `N` elements. It can be used only to initialise a variable declaration.
- `a[i]` accesses an element. The index can have any integer type. An index outside `0` to `N - 1` is a runtime error.
- `a.length` is the number of elements, as an `int`.
- `T[A][B]` is an array of `A` elements of type `T[B]`.
- Arrays are owned values, like structs: assignment transfers ownership of them and `copy()` copies them (§7).
- Arrays can be local variables, fields, parameters (by value or by reference, §7.3) and return values. A reference to an array is written after its size: `int[4]* values`.

### 6.7 Enums

Enums are simple named values. Variants with associated data are an open question (§18).

```nirea
enum State {
    IDLE,
    RUNNING,
    STOPPED
}
```

Enum values are written qualified with the enum name, such as `State.IDLE`, except in the `case` labels of a switch on that enum (§10.1). Enums support `==` and `!=`. They aren't owned, so they're copied like primitive values. Printing an enum value prints its name.

### 6.8 Zero values

Every type has a zero value: `0` for numbers, `false` for `boolean`, `'\0'` for `char`, `""` for `String`, the first value for an enum, all fields zero for a struct and all elements zero for an array.

A local variable declared without an initialiser holds its type's zero value, except for struct types that declare a constructor (§5.4). Struct fields and array elements always start at their zero value, even when their struct type declares a constructor. No value can be read uninitialised.

### 6.9 Equality

`==` and `!=` compare values by their structure:

- Numbers, `char`, `boolean` and enum values compare their values. Floating-point comparison follows IEEE 754, so NaN is not equal to itself.
- Strings compare their bytes.
- Structs compare their fields, in declaration order, with `==`.
- Arrays compare their elements, in order, with `==`.

Both operands must be values of the same type, after implicit conversion for numbers (§6.2).

A reference (a `T*` parameter, `this`, or an alias of either) isn't a value, so using one as an operand of `==` or `!=` is a compile error, whether the other operand is a value or another reference. Readers coming from different languages would expect `==` on references to compare either addresses or values, so Nirea makes both explicit:

- `a is b` checks whether two references point to the same object (§6.10).
- `a.equals(b)` compares the values `a` and `b` refer to, by structure, exactly as `==` compares two values.

`equals` is built in for every struct, array and `String` type. The object it is called on and its argument are both used in place, like `T*` parameters: nothing is copied and nothing changes owner. Either side can be a reference or a value, so `answer.equals("yes")` compares a `String*` parameter with a literal. Fields reached through a reference are values and can be compared with `==` directly (`a.x == b.x`). Interface values have no `equals`.

```nirea
boolean sameContents(Engine* a, Engine* b) {
    return a.equals(b);          // compares the engines' fields
}
```

### 6.10 Identity

`a is b` is `true` when the references `a` and `b` point to the same object in memory.

```nirea
boolean sameEngine(Engine* a, Engine* b) {
    return a is b;
}

void main() {
    Engine first = Engine(100);
    Engine second = Engine(100);
    println(sameEngine(first, first));    // true
    println(sameEngine(first, second));   // false: equal values, different objects
}
```

Both operands of `is` must be references of the same type. A value can't be an operand of `is`. The negation is written `!(a is b)`.

## 7. Ownership and memory safety

Memory safety is one of Nirea's fundamental design principles.

Nirea uses exclusive ownership: every owned object has exactly one owner at a time.

Values of struct, array and `String` types are owned objects. Values of primitive types and enums are plain values: assigning or passing them copies them, and the ownership rules below don't apply to them.

The owner of a value is a local variable, a field of another owned value, or an element of an owned array.

### 7.1 Ownership transfer

Assignment transfers ownership rather than creating an implicit copy.

```nirea
Engine first = createEngine();
Engine second = first;

// first is no longer accessible.
```

The compiler prevents subsequent access through the previous owner.

Ownership is transferred when a local variable is used as:

- the initialiser of a variable declaration: `Engine second = first;`
- the right-hand side of an assignment to a variable, field or array element: `vehicle.engine = first;`
- the value of a `return` statement: `return first;`
- an element of an array initialiser, or the value of a switch-expression arm (§10.1) used in one of these positions.

Passing an object to a value parameter doesn't transfer ownership: the parameter receives a copy (§7.3).

Only a local variable, as a whole, can give its object to a new owner; value parameters count as local variables. An object owned by another object is part of that owner's memory and stays part of it: fields and array elements can't be given to a new owner. Neither can the objects that reference parameters, `this` and aliases refer to, because they belong to someone else. To get an independent object from any of them, call `copy()`:

```nirea
Engine spare = vehicle.engine;          // error: a field stays part of its owner
Engine spare = vehicle.engine.copy();   // ok
```

The compiler follows ownership transfers through all control flow. Using a variable after it has given its object away, or when it may have done so on some path, is a compile-time error. Assigning a new value to such a variable makes it usable again.

```nirea
Engine first = createEngine();
if (ready) {
    Engine second = first;
}
println(first.getPower());   // error: first may no longer own an object
```

A value produced by a call, such as `createEngine()` or `first.copy()`, has no other owner, so it can be used in any of these positions.

### 7.2 Explicit copying

Copying must be requested explicitly.

```nirea
Engine first = createEngine();
Engine second = first.copy();
```

`copy()` creates an independent object.

`copy()` is built in for every struct, array and `String` type, and makes a deep copy: owned fields and elements are copied too. A struct can't declare its own method named `copy` or `equals` (§6.9); type-defined copying and equality are open questions (§18).

### 7.3 Passing values and references

A struct, array or `String` parameter is either a value or a reference.

A **value parameter**, written `Engine engine`, receives a copy of its argument: an object of its own in the called function's memory, owned by the function. The copy is deep, as with `copy()` (§7.2), and the caller's object is unchanged and stays usable. Any object can be passed by value: a local variable, a field, an array element, a reference or `this`. A call result, such as `createEngine()`, is passed without copying, since nothing else owns it.

Inside the function, a value parameter is an owned local variable: it can be modified, assigned, returned, stored in a field or passed on by value. Changes to it aren't visible to the caller.

```nirea
struct Vehicle {
    Engine engine;

    void setEngine(Engine engine) {
        this.engine = engine;      // the field becomes the owner of the copy
    }
}

void main() {
    Engine engine = createEngine();
    Vehicle vehicle;
    vehicle.setEngine(engine);     // setEngine receives a copy
    println(engine.getPower());    // engine is still usable
}
```

This is the one deliberate exception to Nirea's rule against implicit copying (§17): it saves writing `copy()` for every call, and works the same for structs, arrays and strings. To pass an object without copying it, declare the parameter as a reference.

A **reference parameter**, written with `*` after the type, refers to the caller's object where it is in memory. The object stays part of its owner, and ownership doesn't change. The call site looks the same as for a value: `inspect(engine)`.

```nirea
void inspect(Engine* engine) {
    println(engine.getPower());
}

void main() {
    Engine engine = createEngine();

    inspect(engine);

    println(engine.getPower());
}
```

A reference can refer to any object, including one that is part of another: `inspect(vehicle.engine)` and `inspect(engines[0])` refer to the field and the element where they are, inside their owners. Only passing them by value requires `.copy()`.

Through a reference, a function can read and write fields and elements and call methods, and the caller sees the changes. It can't give the object to a new owner or assign to it as a whole, because the object stays with its owner. To keep the value, the function stores a copy:

```nirea
void setEngine(Engine* engine) {
    this.engine = engine.copy();
}
```

`T*` is a reference, not a pointer. It is never null, has no arithmetic, and can only be the type of a parameter, so a reference can't outlive the object it refers to. Only struct, array, `String` and interface types can be passed by reference. Interface parameters must be references (§5.3).

The same object can be passed to several reference parameters of one call, as in `addFrom(counter, counter)`. Parameters of primitive types and enums are always passed by value. They are local copies, and the function can assign to them.

The method receiver `this` is a reference to the object the method was called on. An argument that is a call result, such as `inspect(createEngine())`, lives until the calling statement ends.

Nothing is written at the call site to pass a value or a reference: the parameter declaration decides.

References must not outlive their referenced objects. References exist only as parameters, `this` and aliases (§9), none of which can outlive the object they refer to.

Raw pointers are not exposed in ordinary Nirea code.

### 7.4 Nested ownership

A struct can own other structs.

```nirea
struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}
```

The vehicle owns its engine. The engine cannot simultaneously be owned by another vehicle.

### 7.5 Object replacement

Replacing an owned object transfers ownership to the replacement.

The previous object must be properly destroyed, including its owned resources.

The compiler must prevent invalid outstanding references.

Assigning a new object to a variable, field or array element destroys the object it held before (§7.6), after the new value has been evaluated.

### 7.6 Resource lifecycle

Nirea requires deterministic resource cleanup.

The lifecycle system must account for:

- Nested owned objects.
- Allocated memory.
- File handles.
- Operating-system resources.
- Native resources acquired through C libraries.

Every owned object is destroyed exactly once, automatically, when its owner ends. Destroying an object releases what it owns: a `String` frees its bytes, and a struct or array destroys its fields or elements.

- A local variable, including a value parameter, ends when control leaves its block: at the end of the block, or through `return`, `break` or `continue`. Locals are destroyed in reverse order of declaration. A variable that has transferred ownership of its object doesn't destroy it.
- A field or array element ends with the object that contains it. Fields are destroyed in reverse order of declaration, and array elements from the last to the first.
- An object assigned to a variable, field or element replaces the old object, which is destroyed (§7.5).
- An object that nothing owns, such as a call result used only as an argument for a reference parameter (`inspect(createEngine())`) or as a method receiver, is destroyed at the end of the statement that created it.

A runtime error (§6.4) ends the program immediately, without destroying objects.

User-defined destructors, for structs that hold resources such as file handles, remain to be specified (§18).

## 8. Struct encapsulation

Nirea uses explicit getters and setters.

There are no implicit properties, generated accessors or property syntactic sugar.

```nirea
struct Engine {
    private int power;

    int getPower() {
        return power;
    }

    void setPower(int power) {
        this.power = power;
    }
}
```

Member visibility is public by default, with `private` available for encapsulation.

`private` applies to fields and methods. A private member can be used only inside the methods and constructor of its struct, on any value of that struct type.

Operations remain explicit and visible in source code.

## 9. Aliases

Nirea supports compile-time aliases for expressions.

Aliases shorten repetitive access chains without creating additional variables or runtime objects.

```nirea
void process(Vehicle* vehicle) {
    alias engine = vehicle.engine;
    alias power = engine.getPower();

    println(power);
}
```

The compiler expands aliases into their original expressions.

### Alias rules

1. Aliases can only be declared inside functions.
2. Aliases follow lexical block scope.
3. Aliases cannot be declared globally or as struct members.
4. Aliases introduce no storage.
5. Aliases do not create copies.
6. Aliases do not transfer ownership.
7. Aliases are expanded at compile time.
8. Aliases obey normal type and visibility rules.
9. Aliases cannot bypass ownership restrictions.
10. Aliases cannot introduce implicit operations.

Aliases can refer to mutable locations when the underlying expression is assignable.

A method call returning a value is not automatically an assignable location.

An alias's expression is resolved where the alias is declared, and evaluated again at each use. Following rule 6, an alias can't be the source of an ownership transfer: `Engine other = engine;` is an error when `engine` is an alias.

## 10. Control flow

Nirea retains familiar C/Java-style loops and control statements.

Initial constructs:

- `if`
- `else`
- `for`
- `while`
- `do-while`
- `break`
- `continue`
- `return`

`for` loops have the Java and C form: `for (int i = 0; i < count; i++) { ... }`. Conditions must have type `boolean`; numbers don't convert to `boolean`.

Expressions are evaluated left to right, including function arguments. `&&` and `||` evaluate their right operand only when needed.

Assignment (`=` and compound assignments such as `+=`), `++` and `--` are statements, not expressions, so they can't be nested inside other expressions.

A function with a non-`void` return type must return a value on every path. A path ends with `return` or with a loop that never exits: `while (true)`, `for (;;)` or `do ... while (true)` without a `break`. An `if` with an `else`, or a switch statement, returns on every path when every branch does.

Further rules:

- A local variable can't have the same name as another local variable, parameter or alias that is in scope. A variable declared in a `for` header is scoped to the loop.
- `break` and `continue` outside a loop are errors.
- Statements after `return`, `break` or `continue` are allowed and never run.
- Only calls can be used as expression statements. A call's result may be discarded.

### 10.1 Switch expressions

Nirea uses modern Java/Kotlin-inspired switch expressions.

```nirea
enum State {
    IDLE,
    RUNNING,
    STOPPED
}

String describe(State state) {
    return switch (state) {
        case IDLE -> "Idle";
        case RUNNING -> "Running";
        case STOPPED -> "Stopped";
    };
}
```

Switch can also be used as a statement. A statement arm is a single statement or a block:

```nirea
switch (state) {
    case IDLE -> println("Idle");
    case RUNNING, STOPPED -> {
        println("Busy");
    }
}
```

An arm of a switch expression is a single expression. A `case` can list several values, separated by commas. Case values are enum values (written unqualified), integer, character, boolean or string literals. Qualified enum values (`case State.IDLE`) and named constants aren't allowed as case values. A switch compares its subject with the case values, so the subject must be a value, not a reference (§6.9).

A switch expression of a struct, array or `String` type is used where its result gets an owner (§7.1): as an initialiser, assigned, returned or passed by value. If a branch is an existing object (a variable, field, array element, `this` or alias), the switch can only be used there; elsewhere, for example to call a method on its result, it would act on an implicit copy of that object. A switch whose branches all create new objects, such as constructor calls or string literals, can be used anywhere.

`break` and `continue` inside a switch arm apply to the enclosing loop.

### 10.2 Exhaustiveness

Every switch must be exhaustive.

- Every enum value must be covered.
- Closed sets of variants must be fully covered.
- A `default` branch can handle remaining values.
- Integer switches require a `default` branch.
- Character and string switches require a `default` branch.
- Boolean switches must cover both values unless a default exists.
- Duplicate cases are compilation errors.

There is no implicit fall-through.

Switch expressions produce values and can be used in assignments, returns and other expressions.

## 11. Namespaces

Namespaces organise declarations within source files.

A namespace behaves conceptually like an empty Java class containing static members.

However, namespaces are not classes and cannot be instantiated.

A namespace has exactly one identifier. Dots are not permitted in namespace declarations.

Example:

```nirea
namespace Audio {
    void play() {
        println("Playing audio");
    }

    void stop() {
        println("Stopping audio");
    }

    struct AudioEngine {
        void initialise() {
            // ...
        }
    }
}
```

Usage:

```nirea
void main() {
    Audio.play();

    Audio.AudioEngine engine;
    engine.initialise();
}
```

### Namespace rules

- A namespace has one name.
- Namespace names follow PascalCase.
- Namespace members can include functions, structs, interfaces, enums and constants.
- Namespace functions are accessed through the namespace name.
- Namespaces have no instances.
- Namespaces do not introduce runtime objects.
- Namespaces are independent of filesystem directories.
- Namespace declarations do not imply nested namespace hierarchies.
- Namespaces can't be nested, and each namespace is declared once in a program.
- Inside a namespace, its own members can be used without the namespace prefix. A namespace member can have the same name as a top-level declaration. Inside the namespace, the member takes precedence.
- A namespace can contain `extern "C"` blocks (§14).

Namespaces replace the need for static classes and static methods.

## 12. Source files and imports

Imports reference source files, not namespaces or directories.

The project source root is `src/main/nirea`.

Source files use the `.ni` extension.

### 12.1 Project structure

```text
NireaProject/
├── pom.xml
├── src/
│   ├── main/
│   │   ├── nirea/
│   │   │   ├── Main.ni
│   │   │   └── com/
│   │   │       └── example/
│   │   │           └── Audio.ni
│   │   └── resources/
│   └── test/
│       ├── nirea/
│       └── resources/
└── target/
    ├── generated-c/
    ├── classes/
    └── nirea-app
```

The directory hierarchy organises physical source files.

It does not automatically determine namespace names.

### 12.2 Import syntax

Imports use file paths.

Import from the source root:

```nirea
import com/example/Audio.ni;
```

Relative import:

```nirea
import ../SomeFile.ni;
```

Import from the same directory:

```nirea
import Audio.ni;
```

### 12.3 Import rules

- Imports identify individual source files.
- Import paths use forward slashes.
- Imports can be absolute relative to the source root or relative to the importing file.
- The file extension is explicit.
- Importing a file makes its declarations available.
- Namespace access remains explicit.
- The compiler detects unresolved imports and duplicate declarations.

Importing a file does not automatically merge its namespace with the importing source file.

Resolution:

- A path starting with `./` or `../` is relative to the importing file.
- Any other path is looked up both relative to the importing file's directory and relative to the source root. If both exist and are different files, the import is ambiguous, which is an error. If neither exists, the import is unresolved.
- Imports aren't transitive: a file sees its own declarations and the declarations of the files it imports directly.
- Importing the same file again has no further effect. Circular imports are allowed.
- Every `.ni` file under the source root is part of the program, and top-level names must be unique across the whole program.

## 13. Native compilation

Nirea is designed for ahead-of-time native compilation.

There is no mandatory JVM, garbage collector or managed runtime.

### 13.1 Initial compiler architecture

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

### 13.2 Implementation limits

The first compiler rejects programs beyond these limits:

- Expressions nested more than 1000 levels deep, counting each operator, call, member access and parenthesis.
- Statements and array initialisers nested more than 1000 levels deep.
- Types whose values could be larger than 2147483647 bytes.

### 13.3 Why C as an intermediate language?

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

## 14. C interoperability

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

Struct, array, pointer and callback types at the C boundary remain to be specified (§18).

### Interoperability goals

- Call existing C functions.
- Link against C libraries.
- Use C-compatible structs.
- Support native ABI conventions.
- Expose Nirea functions to C where practical.
- Support system libraries and embedded development.

C interoperability is an explicit safety boundary.

Raw pointers may exist in generated C and the FFI implementation, but they are not part of ordinary Nirea source syntax.

The compiler must ensure that interactions with C do not silently invalidate Nirea's ownership guarantees.

## 15. Libraries

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

## 16. Project management and build structure

Nirea adopts Maven-inspired project organisation.

The initial project convention uses:

- `src/main/nirea` for production source files.
- `src/main/resources` for production resources.
- `src/test/nirea` for test source files.
- `src/test/resources` for test resources.
- `target` for generated files and build output.
- `pom.xml` for project configuration.

Maven-style project coordinates and dependency management are intended to provide a familiar development experience.

Whether Nirea will use Maven directly or introduce a dedicated build tool remains open. The first compiler builds a project from `src/main/nirea` into `target/`, and doesn't read `pom.xml` yet.

## 17. Explicit non-goals

The initial language deliberately excludes:

- Mandatory classes.
- Abstract classes.
- Class and struct inheritance.
- Garbage collection.
- User-visible raw pointers.
- Implicit copying, except for value parameters (§7.3).
- Implicit narrowing conversions.
- Property syntactic sugar.
- Implicit getter/setter generation.
- Implicit fall-through.
- Mandatory virtual machines.
- Complex runtime reflection.
- Mandatory large standard libraries.
- Hidden operations and implicit conversions.

The goal is not to maximise language features, but to make a small set of features work consistently.

## 18. Open design questions

The following decisions require further specification.

| Area        | Question                                                                                                                                    |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| References  | Whether read-only references are needed once types can own heap resources, and whether primitive values can be passed by reference (`int*`) |
| Lifetimes   | How references stored inside structs would be checked, if they are allowed                                                                  |
| Copying     | Type-defined copy and equality behaviour                                                                                                    |
| Destruction | User-defined destructors for structs that hold resources such as file handles                                                               |
| Mutability  | How is object mutation distinguished from field replacement?                                                                                |
| Overloading | Whether functions, methods and constructors can be overloaded                                                                               |
| Generics    | Whether generics are included initially                                                                                                     |
| Enums       | Variants with associated data                                                                                                               |
| Strings     | Text processing beyond the built-in `String` methods, and Unicode text                                                                      |
| Arrays      | Dynamically sized arrays and slices                                                                                                         |
| FFI         | Struct, array, pointer and callback types at the C boundary                                                                                 |
| Modules     | Per-file private declarations, and reopening a namespace in several places                                                                  |
| ABI         | Native calling conventions and data layouts                                                                                                 |
| Errors      | Recoverable error handling                                                                                                                  |
| Concurrency | Ownership and thread-safety rules                                                                                                           |
| Interfaces  | Owned interface values (variables, fields, return values) and static dispatch                                                               |

## 19. Development roadmap

### Phase 1 — Language definition

- Finalise grammar.
- Define primitive types.
- Define functions and structs.
- Define interfaces.
- Define namespaces and imports.
- Define expressions and control flow.
- Specify ownership and lifetime rules.
- Define C interoperability.

### Phase 2 — Initial compiler

- Lexer and parser.
- Abstract syntax tree.
- Name resolution.
- Type checker.
- Ownership checker.
- C code generator.

### Phase 3 — Native toolchain

- GCC/Clang integration.
- Linking.
- Executable generation.
- Build configuration.
- Cross-platform support.

### Phase 4 — Core libraries

- Strings.
- Collections.
- File operations.
- Resource management.
- C interoperability utilities.

### Phase 5 — Systems programming

- Embedded development.
- Native library development.
- Concurrency.
- Performance and optimisation.
- Tooling and package management.

---

## 20. Project identity

**Name:** Nirea

**Category:** Native systems and general-purpose programming language.

**Core idea:**

A small, readable, memory-safe language combining Java-inspired syntax, C's directness, exclusive ownership and native interoperability.

**Guiding principle:**

Explicit code. Simple abstractions. Native execution. Safety by design.

## Appendix A. Grammar

This grammar describes the language as specified in version 0.9. `{ x }` means zero or more repetitions, `[ x ]` means optional, and quoted text is a literal token. `//` and `/* */` comments and whitespace may appear between tokens.

```text
file           = { import } { topDecl } ;
import         = "import" IMPORT_PATH ";" ;
topDecl        = namespaceDecl | decl ;
namespaceDecl  = "namespace" IDENT "{" { decl } "}" ;
decl           = functionDecl | structDecl | interfaceDecl | enumDecl
               | constDecl | externBlock ;

functionDecl   = returnType IDENT "(" [ params ] ")" block ;
params         = param { "," param } ;
param          = type [ "*" ] IDENT ;                    (* "*": reference *)
returnType     = "void" | type ;

structDecl     = "struct" IDENT [ ":" typeName { "," typeName } ] "{" { member } "}" ;
member         = [ "private" ] type IDENT ";"                          (* field *)
               | [ "private" ] returnType IDENT "(" [ params ] ")" block   (* method *)
               | IDENT "(" [ params ] ")" block ;                      (* constructor *)
interfaceDecl  = "interface" IDENT "{" { returnType IDENT "(" [ params ] ")" ";" } "}" ;
enumDecl       = "enum" IDENT "{" IDENT { "," IDENT } [ "," ] "}" ;
constDecl      = "const" type IDENT "=" constValue ";" ;
constValue     = [ "-" ] literal | typeName "." IDENT ;
externBlock    = "extern" STRING "{" { externFunction } "}" ;
externFunction = returnType IDENT "(" [ externParams ] ")" ";" ;
externParams   = "..." | externParam { "," externParam } [ "," "..." ] ;
externParam    = type [ "*" ] IDENT ;                      (* "*" only for String *)

type           = baseType { "[" arraySize "]" } ;
baseType       = primitiveType | "String" | typeName ;
primitiveType  = "boolean" | "char" | "int8" | "int16" | "int" | "int64"
               | "uint8" | "uint16" | "uint" | "uint64" | "float" | "float64" ;
typeName       = IDENT [ "." IDENT ] ;
arraySize      = INTEGER | typeName ;                                  (* integer constant *)

block          = "{" { statement } "}" ;
statement      = block | varDecl | aliasDecl | ifStmt | whileStmt | doStmt | forStmt
               | switchStmt | "break" ";" | "continue" ";" | "return" [ expr ] ";"
               | simpleStmt ";" ;
varDecl        = type IDENT [ "=" initialiser ] ";" ;
initialiser    = expr | "{" initialiser { "," initialiser } [ "," ] "}" ;
aliasDecl      = "alias" IDENT "=" expr ";" ;
simpleStmt     = expr [ assignOp expr | "++" | "--" ] | ( "++" | "--" ) expr ;
assignOp       = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=" ;
ifStmt         = "if" "(" expr ")" statement [ "else" statement ] ;
whileStmt      = "while" "(" expr ")" statement ;
doStmt         = "do" statement "while" "(" expr ")" ";" ;
forStmt        = "for" "(" [ forInit ] ";" [ expr ] ";" [ simpleStmt ] ")" statement ;
forInit        = type IDENT "=" expr | simpleStmt ;
switchStmt     = "switch" "(" expr ")" "{" { switchLabel "->" statement } "}" ;
switchLabel    = "case" caseValue { "," caseValue } | "default" ;
caseValue      = IDENT | [ "-" ] INTEGER | CHARACTER | STRING | "true" | "false" ;

expr           = orExpr ;
orExpr         = andExpr { "||" andExpr } ;
andExpr        = bitOrExpr { "&&" bitOrExpr } ;
bitOrExpr      = bitXorExpr { "|" bitXorExpr } ;
bitXorExpr     = bitAndExpr { "^" bitAndExpr } ;
bitAndExpr     = equalityExpr { "&" equalityExpr } ;
equalityExpr   = relationalExpr { ( "==" | "!=" | "is" ) relationalExpr } ;
relationalExpr = shiftExpr { ( "<" | "<=" | ">" | ">=" ) shiftExpr } ;
shiftExpr      = additiveExpr { ( "<<" | ">>" ) additiveExpr } ;
additiveExpr   = multiplicativeExpr { ( "+" | "-" ) multiplicativeExpr } ;
multiplicativeExpr = unaryExpr { ( "*" | "/" | "%" ) unaryExpr } ;
unaryExpr      = ( "-" | "!" | "~" ) unaryExpr
               | "(" primitiveType ")" unaryExpr                       (* cast *)
               | postfixExpr ;
postfixExpr    = primary { "." IDENT | "[" expr "]" | "(" [ args ] ")" } ;
args           = expr { "," expr } ;
primary        = literal | IDENT | "this" | "(" expr ")" | switchExpr
               | "String" "." IDENT ;                                   (* String.of *)
switchExpr     = "switch" "(" expr ")" "{" { switchLabel "->" expr ";" } "}" ;
literal        = INTEGER | FLOAT | CHARACTER | STRING | "true" | "false" ;
```

`IMPORT_PATH` is the text between `import` and `;`, with surrounding whitespace removed. A statement that starts with `switch` is a switch statement. A statement that starts with a type followed by an identifier is a variable declaration.
