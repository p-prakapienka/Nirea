# Nirea — Language Design Specification

**Version:** 0.2 — Initial Consolidated Design
**Status:** Preliminary Specification

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

Static and dynamic dispatch strategies remain to be specified.

## 6. Primitive types

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

### Conversion rules

- No implicit signed-to-unsigned conversions.
- No implicit narrowing conversions.
- Explicit casts are required for potentially lossy conversions.
- `int` is always 32 bits.
- `uint` is always 32 bits.
- `float` is always binary32.
- `float64` is always binary64.

Integer overflow behaviour must be explicitly specified.

`char` is an 8-bit type. Unicode and UTF-8 string handling belong to library functionality rather than the primitive type system.

## 7. Ownership and memory safety

Memory safety is one of Nirea's fundamental design principles.

Nirea uses exclusive ownership: every owned object has exactly one owner at a time.

### 7.1 Ownership transfer

Assignment transfers ownership rather than creating an implicit copy.

```nirea
Engine first = createEngine();
Engine second = first;

// first is no longer accessible.
```

The compiler prevents subsequent access through the previous owner.

### 7.2 Explicit copying

Copying must be requested explicitly.

```nirea
Engine first = createEngine();
Engine second = first.copy();
```

`copy()` creates an independent object.

Copying semantics, including nested owned objects, must be defined. Types may eventually provide their own copying behaviour.

### 7.3 Non-owning references

Functions can access objects without taking ownership.

```nirea
void inspect(Engine engine) {
    println(engine.getPower());
}

void main() {
    Engine engine = createEngine();

    inspect(engine);

    println(engine.getPower());
}
```

Function parameters are intended to be non-owning by default.

Ownership transfer must be distinguishable from ordinary access through explicit language semantics.

There is no mandatory `borrow` keyword.

References must not outlive their referenced objects.

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

### 7.6 Resource lifecycle

Nirea requires deterministic resource cleanup.

The lifecycle system must account for:

- Nested owned objects.
- Allocated memory.
- File handles.
- Operating-system resources.
- Native resources acquired through C libraries.

Destructor syntax and execution order remain to be specified.

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

Operations remain explicit and visible in source code.

## 9. Aliases

Nirea supports compile-time aliases for expressions.

Aliases shorten repetitive access chains without creating additional variables or runtime objects.

```nirea
void process(Vehicle vehicle) {
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

### 10.2 Exhaustiveness

Every switch must be exhaustive.

- Every enum value must be covered.
- Closed sets of variants must be fully covered.
- A `default` branch can handle remaining values.
- Integer switches require a `default` branch.
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

### 13.2 Why C as an intermediate language?

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

Illustrative syntax:

```nirea
extern "C" {
    int printf(char* format, ...);
}

void main() {
    printf("Hello from Nirea!\n");
}
```

The exact FFI syntax and pointer representation remain to be specified.

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

Whether Nirea will use Maven directly or introduce a dedicated build tool remains open.

## 17. Explicit non-goals

The initial language deliberately excludes:

- Mandatory classes.
- Abstract classes.
- Class and struct inheritance.
- Garbage collection.
- User-visible raw pointers.
- Implicit copying.
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

| Area         | Question                                                     |
| ------------ | ------------------------------------------------------------ |
| Ownership    | Exact syntax for explicit ownership transfer into functions  |
| References   | How are mutable and read-only references distinguished?      |
| Lifetimes    | How are references stored inside structs checked?            |
| Copying      | Shallow, deep or type-defined copy semantics?                |
| Destruction  | Destructor syntax and execution order                        |
| Mutability   | How is object mutation distinguished from field replacement? |
| Constructors | Struct initialisation and construction                       |
| Generics     | Whether generics are included initially                      |
| Enums        | Simple named values or associated data variants              |
| Strings      | Built-in string representation                               |
| FFI          | Exact C declaration and pointer boundary syntax              |
| Modules      | Duplicate declarations and visibility across files           |
| ABI          | Native calling conventions and data layouts                  |
| Errors       | Error handling and exceptional conditions                    |
| Concurrency  | Ownership and thread-safety rules                            |
| Interfaces   | Static and dynamic dispatch semantics                        |
| Imports      | Exact duplicate import and resolution behaviour              |

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
