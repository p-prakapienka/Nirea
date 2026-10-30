# Nirea

Nirea is a statically typed, memory-safe, native programming language for systems programming and general-purpose development. It pairs Java-inspired syntax with a C-inspired programming model.

```nirea
struct Vec2 {
    float x;
    float y;

    float lengthSquared() {
        return x * x + y * y;
    }
}

void main() {
    println("Hello, Nirea!");
}
```

## Key ideas

- Free functions, structs with methods, interfaces and namespaces. No classes or inheritance.
- Exclusive ownership checked by the compiler. No garbage collector and no user-visible raw pointers.
- Ahead-of-time compilation to native code, with direct interoperability with C libraries.
- A small language core, with further functionality in optional libraries.

## Strings

`String` is a growable, owned string. `+` concatenates, `+=` appends, and `==` compares contents.

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

## Ownership and copies

Every struct, array and `String` has exactly one owner, and is freed automatically when its owner goes out of scope.

- Assignment and `return` transfer ownership. The old variable can't be used afterwards.
- `.copy()` makes an explicit deep copy, so both stay usable.
- A value parameter (`Engine engine`) receives a copy automatically. The caller's object is unchanged.
- A reference parameter (`Engine* engine`) works on the caller's object in place.

Strings follow the same rules. The only difference is that a string literal creates a new `String` each time it is evaluated.

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

Using a variable after its object has been transferred is a compile error:

```nirea
Engine engine = Engine(90);
Engine current = engine;
println(engine.power);                   // error: its object was given to a new owner
```

## References

A reference (`T*`) works on an existing object in place, without owning it. Besides parameters, a local variable can be a reference, and a function or method can return one.

- `Engine* engine = vehicle.engine;` declares a reference variable. It is set once, when it is declared, and must refer to an existing object, not a temporary such as `Engine()`.
- A function can return a reference to `this`, a reference parameter, or a part of one, but never to its own local variables.
- A reference never owns its object. `.copy()` gives an independent object, and passing a reference to a value parameter passes a copy.
- An object can be reached in only one way at a time. Reaching it through its owner, or through a new reference, ends the old reference.
- A call can't reach the same object twice through its reference parameters.

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
    Rectangle[3] beds;

    Rectangle* bed(int index) {              // returns a reference into this garden
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
    garden.bed(0).width = 5;                 // changes the first bed inside garden
    Rectangle* second = garden.bed(1);       // a reference to the second bed
    second.width = 2;
    second.height = 6;
    println(garden.beds[0].width);           // 5
    println(garden.beds[1].width * garden.beds[1].height);   // 12
}
```

The compiler rejects references that could outlive their object or reach it in two ways:

```nirea
Engine* engine = vehicle.engine;
vehicle.speed = 50;                      // speed isn't part of the engine
engine.power = 120;
println(vehicle.engine.power);           // reaches the engine through vehicle: engine ends
engine.power = 130;                      // error: the reference ended

Engine* made = Engine();                 // error: a reference can't refer to a temporary
addFrom(counter, counter);               // error: the call reaches counter twice

Engine* make() {
    Engine engine = Engine();
    return engine;                       // error: engine is destroyed when make returns
}
```

## Status

Nirea is at an early stage. The first compiler, `nirea`, implements version 0.10 of the specification. It translates Nirea to C and builds native executables with GCC or Clang.

## Building the compiler

You need CMake 3.16 or later and GCC or Clang.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build -j8      # run the test suite
```

## Using the compiler

```sh
nirea run Hello.ni                    # build and run a single file
nirea build Hello.ni -o hello         # build an executable
nirea build path/to/project           # build a project into target/
nirea check path/to/project           # check without building
```

A project keeps its sources in `src/main/nirea`. `nirea build` writes the generated C to `target/generated-c/` and the executable to `target/nirea-app`. When you build a single file, the compiler also loads the files it imports.

Options:

- `-o FILE` sets the executable path.
- `--emit-c DIR` keeps the generated C in `DIR`.
- `--cc COMPILER` selects the C compiler; the default is `$CC`, then `cc`.
- `-lLIB` and `-LDIR` are passed to the C compiler, for example to link C libraries used through `extern "C"`.

## Documentation

- [Language design specification](docs/language-design.md) (v0.10, preliminary)
- [AGENTS.md](AGENTS.md): guidance for AI coding agents and contributors
