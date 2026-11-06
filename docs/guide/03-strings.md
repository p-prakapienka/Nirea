# 3. Strings

`String` is a growable, owned string of bytes. `+` concatenates, `+=` appends, and `==` compares contents.

```nirea
void main() {
    String name = "Nirea";
    String greeting = "Hello, " + name + "!";
    greeting += " Version " + String.of(9);
    println(greeting);                   // Hello, Nirea! Version 9

    println(greeting.length());          // 23
    println(greeting.substring(7, 12));  // Nirea
    println(greeting.indexOf("!"));      // 12
    greeting.setCharAt(0, 'J');
    println(greeting);                   // Jello, Nirea! Version 9

    println(name == "Nirea");            // true: == compares contents
    String digits = "42";
    println(digits.toInt() + 1);         // 43
}
```

## Methods

| Method                               | Result                                                              |
| ------------------------------------ | ------------------------------------------------------------------- |
| `int length()`                       | the number of bytes                                                 |
| `char charAt(int index)`             | the byte at `index`                                                 |
| `void setCharAt(int index, char c)`  | replaces the byte at `index`                                        |
| `void append(String text)`           | adds `text` at the end                                              |
| `void appendChar(char c)`            | adds one byte at the end                                            |
| `String substring(int from, int to)` | a new string with the bytes from `from` up to, but not including, `to` |
| `int indexOf(String text)`           | the first index of `text`, or `-1`                                  |
| `int toInt()`                        | the text as an `int`                                                |
| `int64 toInt64()`                    | the text as an `int64`                                              |
| `float64 toFloat64()`                | the text as a `float64`                                             |
| `String copy()`                      | an independent copy                                                 |
| `boolean equals(String text)`        | whether both hold the same bytes                                    |

`String.of(value)` turns a number, `boolean`, `char` or enum value into a `String`, with the same text `println` would print. Other values aren't converted implicitly, so write `"count: " + String.of(count)`.

An index outside the string, and text that `toInt`, `toInt64` or `toFloat64` can't read, are runtime errors.

## Strings are objects

A `String` follows the same rules as a struct ([Ownership](07-ownership.md)): assignment transfers it, `.copy()` copies it, and a value parameter receives a copy. The only difference is the literal: each time `"text"` is evaluated, it creates a new `String`.

```nirea
void shout(String text) {                // a value parameter: receives a copy
    text.append("!");
    println(text);                       // hi!
}

void main() {
    String a = "hi";
    shout(a);
    println(a);                          // hi: the caller's string is unchanged

    String b = a;                        // a is no longer usable
    String c = b.copy();                 // b and c are both usable
    c.appendChar('?');
    println(b + " " + c);                // hi hi?

    String twice = "ab";
    twice.append(twice);                 // append takes a copy of its argument
    println(twice);                      // abab
}
```

---

[← 2. Basics](02-basics.md) · [Contents](README.md) · [4. Arrays →](04-arrays.md)
