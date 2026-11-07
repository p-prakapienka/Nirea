# 7. Ownership and memory safety

Memory safety is one of Nirea's fundamental design principles.

Nirea uses exclusive ownership: every owned object has exactly one owner at a time.

Values of struct, array and `String` types are owned objects. Values of primitive types and enums are plain values: assigning or passing them copies them, and the ownership rules below don't apply to them.

The owner of a value is a local variable, a field of another owned value, or an element of an owned array. A variable, field, array element or parameter written without `*` always holds an object of its own, which nothing else uses. To work on an existing object where it is, use a reference, written with `*` ([§7.3](#73-passing-values-and-references), [§9](09-references.md)).

## 7.1 Assignment copies

Assigning an object to a variable, field or array element gives the new owner its own copy. Both objects stay usable, and changing one doesn't change the other:

```nirea
Engine first = createEngine();
Engine second = first;            // second is a copy of first
second.setPower(200);
println(first.getPower());        // first is unchanged
```

The same holds for:

- the initialiser of a variable declaration: `Engine second = first;`
- the right-hand side of an assignment to a variable, field or array element: `vehicle.engine = first;`
- the value of a `return` statement: `return first;`
- an argument for a value parameter ([§7.3](#73-passing-values-and-references));
- an element of an array initialiser, or the value of a switch-expression arm ([§10.1](10-control-flow.md#101-switch-expressions)) used in one of these positions.

Any object can be copied: a local variable, a parameter, `this`, a field, an array element, or the object a reference refers to. A copy is deep: owned fields and array elements are copied too.

```nirea
Engine spare = vehicle.engine;    // a copy of the vehicle's engine
vehicle.engine = spare;           // the vehicle gets a copy of spare
```

A value produced by a call that returns a value, such as `createEngine()`, by a constructor, a string literal or a string concatenation is a new object that nothing else owns, so it is never copied.

## 7.2 Moving instead of copying

When the object assigned is held by a local variable, including a value parameter, that isn't used afterwards, the object moves to its new owner instead of being copied. Moving costs nothing, and it can't change what a program does, because nothing sees the variable again. For example, `return engine;` never copies a local `engine`, and `Engine second = first;` doesn't copy when `first` isn't used later.

The variable counts as used afterwards if, on any path, it is used later, including in a later iteration of a loop, if a reference variable that reaches its object is used later ([§9.3](09-references.md#93-reading-and-changing)), or if the same statement still uses it, for example as the object a call is made on. The compiler follows this through all control flow, as for initialisation ([§6.8](06-types.md#68-zero-values-and-initialisation)). Objects held by fields, array elements or references are always copied.

To be sure that a large object isn't copied, refer to it with a reference instead ([§9](09-references.md)).

There is no built-in `copy()` method: assigning an object to a new variable copies it. A struct can declare its own method named `copy`. Type-defined copying, for example for structs that hold resources, is an open question ([§18](18-open-questions.md)).

## 7.3 Passing values and references

A struct, array or `String` parameter is either a value or a reference.

A **value parameter**, written `Engine engine`, receives its own object, exactly as a variable initialised with the argument would ([§7.1](#71-assignment-copies)): a copy of the argument, in the called function's memory, owned by the function. The caller's object is unchanged and stays usable. Any object can be passed by value: a local variable, a field, an array element, a reference or `this`. A call result that is a new object, such as `createEngine()`, is passed without copying, and so is a local variable that isn't used afterwards ([§7.2](#72-moving-instead-of-copying)).

Inside the function, a value parameter is an owned local variable: it can be modified, assigned, returned, stored in a field or passed on by value. Changes to it aren't visible to the caller.

```nirea
struct Vehicle {
    Engine engine;

    void setEngine(Engine engine) {
        this.engine = engine;      // the field gets the engine
    }
}

void main() {
    Engine engine = createEngine();
    Vehicle vehicle = Vehicle();
    vehicle.setEngine(engine);     // setEngine receives a copy
    println(engine.getPower());    // engine is still usable
}
```

To pass an object without copying it, declare the parameter as a reference.

A **reference parameter**, written with `*` after the type, refers to the caller's object where it is in memory. The object stays part of its owner, and ownership doesn't change. The call site looks the same as for a value: `inspect(engine)`.

```nirea
void inspect(Engine* engine) {
    println(engine.getPower());
}

void main() {
    Engine engine = createEngine();

    inspect(engine);

    println(engine.getPower());
}
```

A reference can refer to any object, including one that is part of another: `inspect(vehicle.engine)` and `inspect(engines[0])` refer to the field and the element where they are, inside their owners.

Through a reference, a function can read and write fields and elements and call methods, and the caller sees the changes. It can't assign to the object as a whole, because the object stays with its owner. Assigning the object to a variable or field stores a copy:

```nirea
void setEngine(Engine* engine) {
    this.engine = engine;          // stores a copy of the caller's engine
}
```

`T*` is a reference, not a pointer. It is never null and has no arithmetic. Besides parameters, it can be the type of a local variable or a return value ([§9](09-references.md)), but never of a field or array element, so a reference can't outlive the object it refers to. Only struct, array, `String` and interface types can be passed by reference. Interface parameters must be references ([§5.3](05-syntax.md#53-interfaces)).

A call can receive the same object more than once, as the object it is called on and as arguments for reference parameters, as long as it changes none of them ([§9.3](09-references.md#93-reading-and-changing)). Parameters of primitive types and enums are always passed by value. They are local copies, and the function can assign to them.

The method receiver `this` is a reference to the object the method was called on. An argument that is a call result, such as `inspect(createEngine())`, lives until the calling statement ends.

Nothing is written at the call site to pass a value or a reference: the parameter declaration decides.

References must not outlive their referenced objects. [Section 9](09-references.md) describes reference variables and functions that return references, and the rules that keep every reference valid.

Raw pointers are not exposed in ordinary Nirea code.

## 7.4 Nested ownership

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

## 7.5 Object replacement

Assigning a new object to an owner replaces the object it held.

The previous object must be properly destroyed, including its owned resources.

The compiler must prevent invalid outstanding references.

Assigning a new object to a variable, field or array element destroys the object it held before ([§7.6](#76-resource-lifecycle)), after the new value has been evaluated.

## 7.6 Resource lifecycle

Nirea requires deterministic resource cleanup.

The lifecycle system must account for:

- Nested owned objects.
- Allocated memory.
- File handles.
- Operating-system resources.
- Native resources acquired through C libraries.

Every owned object is destroyed exactly once, automatically, when its owner ends. Destroying an object releases what it owns: a `String` frees its bytes, and a struct or array destroys its fields or elements.

- A local variable, including a value parameter, ends when control leaves its block: at the end of the block, or through `return`, `break` or `continue`. Locals are destroyed in reverse order of declaration. A variable whose object moved to a new owner ([§7.2](#72-moving-instead-of-copying)) doesn't destroy it.
- A field or array element ends with the object that contains it. Fields are destroyed in reverse order of declaration, and array elements from the last to the first.
- An object assigned to a variable, field or element replaces the old object, which is destroyed ([§7.5](#75-object-replacement)).
- An object that nothing owns, such as a call result used only as an argument for a reference parameter (`inspect(createEngine())`) or as a method receiver, is destroyed at the end of the statement that created it.

A runtime error ([§6.4](06-types.md#64-arithmetic-and-runtime-errors)) ends the program immediately, without destroying objects.

User-defined destructors, for structs that hold resources such as file handles, remain to be specified ([§18](18-open-questions.md)).

---

[← 6. Types](06-types.md) · [Contents](README.md) · [8. Struct encapsulation →](08-encapsulation.md)
