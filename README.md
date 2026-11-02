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
- No hidden values: objects are created explicitly, as in `Circle circle = Circle();`, and a local variable must be assigned before it is used.
- Ahead-of-time compilation to native code, with direct interoperability with C libraries.
- A small language core, with further functionality in optional libraries.

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

## Arrays

Arrays work as in Java. The type `T[]` has no size: the length is fixed when the array is created, with `T[n]` or from a list of values. Elements start at their zero value, `a.length` gives the length, and an index outside the array is a runtime error. Arrays are objects, like structs and strings: assignment transfers them and `.copy()` copies them.

```nirea
struct Team {
    String[] players;

    Team(int size) {
        players = String[size];              // array fields start empty, so create them
    }
}

int total(int[]* values) {                   // a reference: the array isn't copied
    int sum = 0;
    for (int i = 0; i < values.length; i++) {
        sum += values[i];
    }
    return sum;
}

void main() {
    int[] scores = int[5];                   // 5 elements, all 0
    scores[0] = 10;
    scores[4] = 7;
    println(scores.length);                  // 5
    println(total(scores));                  // 17

    String[] names = {"Ann", "Bob"};         // created from values
    println(names[1]);                       // Bob

    int size = names.length * 2;
    float[] samples = float[size];           // the length can be computed
    println(samples.length);                 // 4

    int[][] grid = int[2][3];                // 2 rows of 3 ints
    grid[1][2] = 9;
    grid[0] = int[1];                        // rows can have different lengths
    println(grid[0].length);                 // 1

    Team team = Team(3);
    team.players[0] = "Ann";
    println(team.players.length);            // 3

    int[] backup = scores.copy();            // explicit copy: both stay usable
    int[] moved = scores;                    // assignment transfers ownership
    println(backup[0] + moved[4]);           // 17
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

The compiler rejects references that could outlive their object or reach it in two ways:

```nirea
Engine* engine = vehicle.engine;
Engine owned = engine;                   // error: a reference doesn't own its object; use engine.copy()
engine = other.engine;                   // error: a reference is set when it is declared
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

Nirea is at an early stage. The first compiler, `nirea`, implements version 0.12 of the specification. It translates Nirea to C and builds native executables with GCC or Clang.

## Building the compiler

You need CMake 3.16 or later and GCC or Clang.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build -j8      # run the test suite
```

### Windows

Install CMake and a MinGW-w64 toolchain: GCC (for example from MSYS2 or WinLibs) or Clang (for example llvm-mingw). With GCC:

```sh
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
ctest --test-dir build -j8
```

The compiler is `build\nirea.exe`. It needs GCC or Clang on `PATH` to build programs too, because it compiles the C it generates with them. Microsoft's C compiler (`cl.exe`) isn't supported.

## Using the compiler

```sh
nirea run Hello.ni                    # build and run a single file
nirea build Hello.ni -o hello         # build an executable
nirea build path/to/project           # build a project into target/
nirea check path/to/project           # check without building
```

A project keeps its sources in `src/main/nirea`. `nirea build` writes the generated C to `target/generated-c/` and the executable to `target/nirea-app` (`nirea-app.exe` on Windows, where executables always get `.exe`). When you build a single file, the compiler also loads the files it imports.

Options:

- `-o FILE` sets the executable path.
- `--emit-c DIR` keeps the generated C in `DIR`.
- `--cc COMPILER` selects the C compiler. The default is `$CC`, then `cc`; on Windows, `$CC`, then `gcc`, then `clang`.
- `-lLIB` and `-LDIR` are passed to the C compiler, for example to link C libraries used through `extern "C"`.

## Documentation

- [Language design specification](docs/language-design.md) (v0.12, preliminary)
- [AGENTS.md](AGENTS.md): guidance for AI coding agents and contributors
