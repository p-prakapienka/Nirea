# Enums

An enum is a set of named values. Its values are written with the enum's name, `Light.RED`, except in the `case` labels of a switch on that enum.

```nirea
enum Light {
    RED,
    YELLOW,
    GREEN
}

Light next(Light light) {
    return switch (light) {
        case RED -> Light.GREEN;
        case GREEN -> Light.YELLOW;
        case YELLOW -> Light.RED;
    };
}

void main() {
    Light light = Light.RED;
    light = next(light);
    println(light);                          // GREEN
    println(light == Light.GREEN);           // true

    switch (light) {
        case RED -> println("stop");
        case YELLOW, GREEN -> println("go"); // go
    }

    Light[] lights = Light[2];
    println(lights[0]);                      // RED: the zero value is the first value
}
```

- A switch on an enum must cover every value, or have a `default` branch. A missing value is a compile error, so adding a value to an enum shows you every switch to update.
- Enum values are plain values, like numbers: assigning them copies them.
- `println` and `String.of` give the value's name.
- Enum values with data attached are still an open question.

---

[← Structs and interfaces](structs.md) · [Contents](README.md) · [Ownership →](ownership.md)
