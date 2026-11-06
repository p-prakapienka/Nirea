# 5. Basic syntax

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

## 5.1 Functions

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

Functions, methods and constructors can't be overloaded: each name is declared once in its scope ([§18](18-open-questions.md)). Declarations can be used regardless of their order in the file: functions can call functions declared later, and recursion is allowed. The same holds for structs, interfaces, enums and constants.

A program has exactly one top-level `main` function, declared as `void main()` or `int main()`. The value returned by `int main()` is the process exit status.

## 5.2 Structs

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

Fields are stored inline, in declaration order. An array field holds its length and a pointer to its elements, which the struct owns ([§6.6](06-types.md#66-arrays)). A struct can't contain itself, directly or through other structs; it can contain an array of itself, as in `struct Node { Node[] children; }`.

Inside a method, fields and other methods of the same struct can be used by name, or through `this`: `this.x` and `x` are the same field. `this` is a non-owning reference to the value the method was called on ([§7.3](07-ownership.md#73-passing-values-and-references)).

## 5.3 Interfaces

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

Interface types can be used only as references, written `Drawable*`: as parameters ([§7.3](07-ownership.md#73-passing-values-and-references)), reference variables and return values ([§9](09-references.md)). An interface parameter is a non-owning reference to a struct value that implements the interface, and calls through it use dynamic dispatch. Any struct that implements the interface can be passed for it, and an interface-typed parameter can itself be passed on to another parameter of the same interface type.

```nirea
void render(Drawable* shape) {
    shape.draw();
}

void main() {
    Circle circle = Circle();
    circle.radius = 2.0;
    render(circle);
}
```

Owned interface values, that is variables, fields, array elements and return values of an interface type without `*`, are not supported yet ([§18](18-open-questions.md)).

## 5.4 Constructors

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

All fields hold their zero value ([§6.8](06-types.md#68-zero-values-and-initialisation)) before the constructor body runs, so no field is ever read uninitialised. A constructor can set private fields.

A struct that declares no constructor can be created with `T()`, which produces a zero-initialised value. For a struct that declares a constructor, `T(...)` always calls that constructor. A local variable is never created implicitly: `Vec2 v;` only declares `v`, which must be assigned before it is used ([§6.8](06-types.md#68-zero-values-and-initialisation)). Storage owned by another value is the exception: fields and array elements of a struct type start at their zero value, even when the struct declares a constructor ([§6.8](06-types.md#68-zero-values-and-initialisation)).

## 5.5 Constants

Constants are declared with `const` at the top level of a file or inside a namespace.

```nirea
const int MAX_BUFFER_SIZE = 1024;
const String APP_NAME = "Nirea";
```

A constant has a primitive, enum or `String` type. Its initialiser is a literal, a negated numeric literal or an enum value. A `String` constant is a named literal: each use creates a new `String` object, like writing the literal ([§6.5](06-types.md#65-strings)).

There are no global variables. Mutable state lives in local variables, in the values they own, and in global objects ([§5.6](#56-global-objects)).

## 5.6 Global objects

A global object is a struct value that belongs to the program: it is created before `main` runs and destroyed after `main` returns. It is declared at the top level of a file or inside a namespace, with a struct type and an initialiser, and its name follows the constant convention ([§4](04-naming.md)):

```nirea
interface Repository {
    void save(User user);
    User find(int id);
}

struct UserStore : Repository {
    private User[] users;
    private int count;

    UserStore() {
        users = User[16];
    }

    void save(User user) { ... }
    User find(int id) { ... }
}

UserStore USERS = UserStore();

void main() {
    USERS.save(User("Ann"));
    User ann = USERS.find(1);            // a copy
}
```

A global object is used only by calling its methods: `USERS.save(user)`. It can't be used as a value, so it can't be assigned, given to a new owner, copied, compared or passed as an argument, and its fields can't be used through its name, even when they are public. Its methods that return a reference ([§9.2](09-references.md#92-returning-references)) can't be called on it. Inside its methods, its fields are used through `this`, as for any struct value. A struct type with a global object can still have ordinary local values.

To choose an implementation, for example in tests, a global object can forward each call to another one:

```nirea
struct Users : Repository {
    void save(User user) {
        if (SETTINGS.testing()) {
            FAKE_USERS.save(user);
        } else {
            USERS.save(user);
        }
    }
    ...
}

Users REPOSITORY = Users();
```

Global objects are created in dependency order: before a global object's initialiser runs, every global object it can reach ([§9.3](09-references.md#93-reading-and-changing)) has been created. An initialiser that can reach its own global object, directly or through others, is an error. They are destroyed after `main` returns, in the reverse order. A runtime error ends the program without destroying them ([§6.4](06-types.md#64-arithmetic-and-runtime-errors)).

Global objects are shared by the whole program, so they are the one way to reach an object that isn't passed to a function or owned by it. [Section 9.3](09-references.md#93-reading-and-changing) describes how the compiler keeps references into them valid. Sharing them between threads is an open question ([§18](18-open-questions.md)).

---

[← 4. Naming conventions](04-naming.md) · [Contents](README.md) · [6. Types →](06-types.md)
