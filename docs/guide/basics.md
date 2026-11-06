# Basics

## Variables and types

A variable is declared with its type:

```nirea
void main() {
    int count = 3;
    float64 price = 9.5;
    boolean ready = true;
    char letter = 'N';
    String name = "Nirea";
    println(name);                       // Nirea
}
```

The primitive types:

| Type                                | Values                                     |
| ----------------------------------- | ------------------------------------------ |
| `boolean`                           | `true` or `false`                          |
| `char`                              | one byte, 0 to 255                         |
| `int8`, `int16`, `int`, `int64`     | signed integers of 8, 16, 32 and 64 bits   |
| `uint8`, `uint16`, `uint`, `uint64` | unsigned integers of 8, 16, 32 and 64 bits |
| `float`, `float64`                  | 32-bit and 64-bit floating point           |

`String`, arrays and structs are objects. They have their own pages: [Strings](strings.md), [Arrays](arrays.md) and [Structs and interfaces](structs.md).

## Initialisation

Nothing gets a value implicitly. Objects are created explicitly: a struct with its constructor or `T()`, an array with `T[n]` or from values, a string from a literal. A declaration without an initialiser only declares the variable. It has no value until one is assigned, so declaration and assignment can be split, and the compiler rejects any use before the assignment, on every path. Only struct fields and array elements start at their zero value.

```nirea
struct Circle {
    float radius;

    Circle(float radius) {
        this.radius = radius;
    }
}

struct Point {
    int x;
    int y;
}

void main() {
    Circle circle = Circle(2.0);             // created by its constructor
    Point origin = Point();                  // no constructor: every field is zero
    println(origin.x);                       // 0

    Circle chosen;                           // declared, no value yet
    boolean big = true;
    if (big) {
        chosen = Circle(10.0);               // assigned on every path
    } else {
        chosen = Circle(1.0);
    }
    println(chosen.radius);                  // 10.0

    int count;                               // numbers need a value too
    count = 3;
    println(count + 1);                      // 4

    int[] values = int[3];                   // array elements start at zero
    println(values[2]);                      // 0
}
```

Using a variable before it has a value is a compile error:

```nirea
Circle circle;
circle.radius = 2.0;                     // error: 'circle' has no value yet

int count;
count++;                                 // error: 'count' has no value yet

String status;
if (ready) {
    status = "ready";
}
println(status);                         // error: 'status' may have no value on some path
```

## Numbers

Arithmetic needs both operands to have the same type. A smaller type converts to a larger one implicitly, when every value fits. Anything that could lose information needs a cast, written as in Java:

```nirea
void main() {
    int8 small = 100;
    int count = 1000;
    int sum = small + count;                 // int8 converts to int
    println(sum);                            // 1100

    float64 price = 2.9;
    int whole = (int) price;                 // a cast truncates toward zero
    println(whole);                          // 2
    uint8 low = (uint8) 300;                 // an integer cast keeps the low bits
    println(low);                            // 44

    println(7 / 2);                          // 3
    println(-7 % 2);                         // -1
    println(1.0 / 4.0);                      // 0.25
    println(0x2A + 0b10 + 1_000);            // 1044
}
```

These are runtime errors: integer overflow, integer division by zero, a shift by a negative amount or by the type's width or more, and a float that doesn't fit the integer type it is cast to. A runtime error stops the program at once:

```nirea
int max = 2147483647;
max++;                                   // runtime error: integer overflow
```

## Functions

Functions don't need a struct around them. They can be declared in any order, and they can call themselves:

```nirea
int add(int a, int b) {
    return a + b;
}

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

void main() {
    println(add(2, 3));                      // 5
    println(factorial(5));                   // 120
}
```

Each name is declared once: functions can't be overloaded. A function that returns a value must return on every path.

## Control flow

`if`, `else`, `while`, `do`-`while`, `for`, `break` and `continue` work as in Java and C. Conditions must be `boolean`: numbers don't convert to `boolean`. Assignments, `++` and `--` are statements, not expressions.

```nirea
void main() {
    int total = 0;
    for (int i = 1; i <= 10; i++) {
        if (i % 2 == 0) {
            continue;                        // skip even numbers
        }
        total += i;
    }
    println(total);                          // 25

    int n = 0;
    while (true) {
        n++;
        if (n == 3) {
            break;
        }
    }
    println(n);                              // 3

    do {
        n--;
    } while (n > 0);
    println(n);                              // 0
}
```

## Switch

A switch can be an expression or a statement. There is no fall-through, and a `case` can list several values. A switch on numbers, characters or strings needs a `default` branch. A switch on an enum must cover every value ([Enums](enums.md)).

```nirea
String size(int count) {
    return switch (count) {
        case 0 -> "none";
        case 1, 2, 3 -> "a few";
        default -> "many";
    };
}

void main() {
    println(size(2));                        // a few
    println(size(7));                        // many

    String command = "stop";
    switch (command) {
        case "start" -> println("starting");
        case "stop", "halt" -> {
            println("stopping");             // stopping
        }
        default -> println("unknown");
    }
}
```

## Constants

Constants are declared with `const` at the top level of a file or in a namespace. They have a primitive, enum or `String` type, and a literal value:

```nirea
const int MAX_PLAYERS = 4;
const String APP_NAME = "Nirea";

void main() {
    println(APP_NAME + " allows " + String.of(MAX_PLAYERS) + " players");   // Nirea allows 4 players
}
```

There are no global variables. State lives in local variables, in the objects they own, and in [global objects](global-objects.md).

## Printing

`print(value)` and `println(value)` write one value: a number, `boolean`, `char`, enum value or `String`. `println()` prints an empty line. To print several values together, build a string with `+` and `String.of` ([Strings](strings.md)).

## Comments

`//` comments run to the end of the line, and `/* */` comments can span lines, as in Java and C.

---

[← Getting started](getting-started.md) · [Contents](README.md) · [Strings →](strings.md)
