# 8. References

A reference (`T*`) works on an existing object in place, without owning it. Besides parameters, a local variable can be a reference, and a function or method can return one.

- `Engine* engine = vehicle.engine;` declares a reference variable. It is set once, when it is declared, and must refer to an existing object, not a temporary such as `Engine()`.
- A function can return a reference to `this`, a reference parameter, or a part of one, but never to its own local variables.
- A reference never owns its object. `.copy()` gives an independent object, and passing a reference to a value parameter passes a copy.
- Any number of references can read an object at once. Changing the object ends the other references to it, and a reference used to change an object must be the only way to it while it is in use. The compiler works out what changes what; nothing is marked.
- A call can receive the same object more than once if it only reads it, but not if it changes it.

## Reference variables

A reference variable gives a short name to an object, wherever it is, without copying it:

```nirea
interface Shape {
    int area();
}

struct Square : Shape {
    int side;

    int area() {
        return side * side;
    }
}

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;
}

struct Garage {
    Vehicle[] vehicles;

    Garage(int size) {
        vehicles = Vehicle[size];
    }
}

void tune(Engine* engine) {                  // a reference parameter
    engine.power = engine.power + 10;
}

void main() {
    Garage garage = Garage(3);

    Engine* engine = garage.vehicles[1].engine;  // a short name for a nested object
    engine.power = 100;
    tune(engine);                            // passing it on keeps it usable
    println(engine.power);                   // 110
    Engine spare = engine.copy();            // an independent copy

    for (int i = 0; i < garage.vehicles.length; i++) {
        Vehicle* vehicle = garage.vehicles[i];   // a new reference on every iteration
        vehicle.speed = (i + 1) * 10;
    }
    println(garage.vehicles[2].speed);       // 30
    println(spare.power);                    // 110

    String name = "Rover";
    String* text = name;                     // strings work the same way
    text.append(" 2");
    println(name);                           // Rover 2

    Square square = Square();
    square.side = 4;
    Shape* shape = square;                   // a reference to an interface
    println(shape.area());                   // 16
}
```

## Returning references

Methods that return `this` let calls be chained:

```nirea
struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;
    String name;

    Vehicle* setSpeed(int speed) {
        this.speed = speed;
        return this;
    }

    Vehicle* setName(String name) {
        this.name = name;
        return this;
    }
}

void main() {
    Vehicle vehicle = Vehicle();
    vehicle.setSpeed(60).setName("Rover");   // both calls change vehicle
    println(vehicle.name);                   // Rover

    Engine* engine = vehicle.engine;         // refers to the engine inside vehicle
    engine.power = 120;
    println(vehicle.engine.power);           // 120
}
```

Chaining also works on a temporary object, which lasts until the end of the statement. A getter can return a reference into its struct:

```nirea
struct Rectangle {
    int width;
    int height;
}

struct Builder {
    int width;
    int height;

    Builder* setWidth(int width) {           // returns a reference to this builder
        this.width = width;
        return this;
    }

    Builder* setHeight(int height) {
        this.height = height;
        return this;
    }

    Rectangle build() {                      // returns a new Rectangle
        Rectangle rectangle = Rectangle();
        rectangle.width = width;
        rectangle.height = height;
        return rectangle;
    }
}

struct Garden {
    Rectangle[] beds;

    Garden() {
        beds = Rectangle[3];                 // an array of 3 rectangles
    }

    Rectangle* getBed(int index) {           // returns a reference into this garden
        return beds[index];
    }
}

void main() {
    Rectangle rectangle = Builder()          // a temporary builder, destroyed after this statement
        .setWidth(3)
        .setHeight(4)
        .build();
    println(rectangle.width * rectangle.height);   // 12

    Garden garden = Garden();
    garden.getBed(0).width = 5;              // changes the first bed inside garden
    Rectangle* second = garden.getBed(1);    // a reference to the second bed
    second.width = 2;
    second.height = 6;
    println(garden.beds[0].width);           // 5
    println(garden.beds[1].width * garden.beds[1].height);   // 12
}
```

## Reading and changing

Any number of references can read an object at the same time. Changing the object ends the other references to it, so none of them can see it change, or point at something that was replaced. A reference that is used to change its object must be the only way to it while it is in use. Nothing is marked: the compiler works out which methods and functions change what.

```nirea
struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;
}

int total(Engine* a, Engine* b) {            // only reads both engines
    return a.power + b.power;
}

void main() {
    Vehicle vehicle = Vehicle();
    vehicle.engine.power = 50;

    Engine* a = vehicle.engine;              // a and b only read
    Engine* b = vehicle.engine;
    println(a.power + b.power);              // 100
    println(vehicle.engine.power);           // 50: reading through the owner is fine too
    println(total(vehicle.engine, vehicle.engine));   // 100: total only reads

    vehicle.engine.power = 70;               // a change: a and b end here
    Engine* engine = vehicle.engine;         // a new reference after the change
    println(engine.power);                   // 70
}
```

Parts that don't overlap are independent: changing `vehicle.speed` doesn't end a reference to `vehicle.engine`, and `engines[0]` and `engines[1]` are separate. An element with a computed index, such as `engines[i]`, could be any element, so it overlaps them all.

## What the compiler rejects

The compiler rejects references that could outlive their object, or that could see it change while they use it:

```nirea
Engine* engine = vehicle.engine;
Engine owned = engine;                   // error: a reference doesn't own its object; use engine.copy()
engine = other.engine;                   // error: a reference is set when it is declared
vehicle.speed = 50;                      // speed isn't part of the engine
engine.power = 120;
println(vehicle.engine.power);           // engine changes the engine, so any other use ends it
engine.power = 130;                      // error: the reference ended

Engine* made = Engine();                 // error: a reference can't refer to a temporary
addFrom(counter, counter);               // error: addFrom changes counter, so it can't receive it twice

Engine* make() {
    Engine engine = Engine();
    return engine;                       // error: engine is destroyed when make returns
}
```

[Global objects](09-global-objects.md) add one more rule for references into a global object.

---

[← 7. Ownership](07-ownership.md) · [Contents](README.md) · [9. Global objects →](09-global-objects.md)
