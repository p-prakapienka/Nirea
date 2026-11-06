# Ownership

Nirea has no garbage collector. Instead, every struct, array and `String` has exactly one owner, and is freed automatically when its owner goes out of scope. The compiler checks the rules, so a program can't use an object after it is freed, or free it twice.

- Assignment and `return` transfer ownership. The old variable can't be used afterwards.
- `.copy()` makes an explicit deep copy, so both stay usable.
- A value parameter (`Engine engine`) receives a copy automatically. The caller's object is unchanged.
- A reference parameter (`Engine* engine`) works on the caller's object in place ([References](references.md)).

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

    Engine spare = engine.copy();        // explicit copy: both stay usable
    Engine current = engine;             // assignment transfers ownership
    println(spare.power);                // 190
    println(current.power);              // 190

    String a = "abc";
    String b = a;                        // a is no longer accessible
    String c = b.copy();                 // b and c are both usable
    println(b + c);                      // abcabc
}
```

Using a variable after its object has been transferred is a compile error, even if it happened on only one path:

```nirea
Engine engine = Engine(90);
Engine current = engine;
println(engine.power);                   // error: its object was given to a new owner

Engine first = Engine(1);
if (ready) {
    Engine second = first;
}
println(first.power);                    // error: first may have given its object away
```

Assigning a new object to the variable makes it usable again.

## Parts of an object

A field or array element is part of its owner, so it can't be given away. Copy it instead:

```nirea
Engine spare = vehicle.engine;           // error: a field stays part of its owner
Engine spare = vehicle.engine.copy();    // ok
```

A function receives and returns objects the same way:

```nirea
struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;

    void setEngine(Engine engine) {      // receives a copy
        this.engine = engine;            // the field becomes its owner
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
    println(engine.power);               // 150: engine is still usable
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

[← Enums](enums.md) · [Contents](README.md) · [References →](references.md)
