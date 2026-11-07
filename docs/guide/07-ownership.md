# 7. Ownership

Nirea has no garbage collector. Instead, every struct, array and `String` has exactly one owner, and is freed automatically when its owner goes out of scope. The compiler checks the rules, so a program can't use an object after it is freed, or free it twice.

- A variable without `*` always holds an object of its own. Assigning an object to it gives it a copy, so both stay usable.
- The same holds for fields, array elements, `return` and value parameters (`Engine engine`).
- When the old variable isn't used again, the object moves instead of being copied, which costs nothing.
- To work on an existing object in place, without a copy, use a reference (`Engine* engine`). See [References](08-references.md).

Numbers, `boolean`, `char` and enum values aren't owned: they are copied like in any other language.

```nirea
struct Engine {
    int power;

    Engine(int power) {
        this.power = power;
    }
}

void tune(Engine engine) {               // value parameter: receives a copy
    engine.power = engine.power + 10;
}

void boost(Engine* engine) {             // reference: works on the caller's engine
    engine.power = engine.power + 100;
}

void main() {
    Engine engine = Engine(90);
    tune(engine);                        // passes a copy
    println(engine.power);               // 90
    boost(engine);                       // changes engine in place
    println(engine.power);               // 190

    Engine spare = engine;               // a copy: both stay usable
    spare.power = 1;
    println(engine.power);               // 190
    println(spare.power);                // 1

    String a = "abc";
    String b = a;                        // strings work the same way
    b.append("d");
    println(a + " " + b);                // abc abcd
}
```

There is no `copy()` method: assignment is how you copy.

## Copies and moves

Copying a big object, such as a large array, takes time. The compiler avoids it when it can: if the old variable isn't used again, its object moves to the new owner and nothing is copied. Returning a local variable never copies it.

```nirea
int[] squares(int count) {
    int[] table = int[count];
    for (int i = 0; i < count; i++) {
        table[i] = i * i;
    }
    return table;                        // moves: table ends here
}

void main() {
    int[] table = squares(1000);
    int[] saved = table;                 // moves: table isn't used again
    println(saved[10]);                  // 100
}
```

A variable that is used later, on any path or in a later loop iteration, is copied instead. To be sure that a big object isn't copied, use a reference to it: `int[]* view = table;`.

## Parts of an object

A field or array element is part of its owner. Assigning it to a variable gives a copy, and the original stays where it is:

```nirea
struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;

    void setEngine(Engine engine) {      // receives a copy
        this.engine = engine;            // the field takes it over
    }
}

Engine makeEngine(int power) {
    Engine engine = Engine();
    engine.power = power;
    return engine;                       // the caller becomes the owner
}

void main() {
    Engine engine = makeEngine(150);
    Vehicle vehicle = Vehicle();
    vehicle.setEngine(engine);
    Engine spare = vehicle.engine;       // a copy of the field
    spare.power = 1;
    println(engine.power);               // 150
    println(vehicle.engine.power);       // 150
}
```

## When objects are destroyed

- A local variable's object is destroyed when the variable goes out of scope: at the end of its block, or through `return`, `break` or `continue`.
- Fields and array elements are destroyed with the object that holds them.
- Assigning a new object to a variable, field or element destroys the old object.
- A temporary object, such as a call result used only for a method call, is destroyed at the end of the statement.

Destructors of your own, for example to close a file, aren't available yet.

---

[← 6. Enums](06-enums.md) · [Contents](README.md) · [8. References →](08-references.md)
