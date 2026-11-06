# 7. Ownership and memory safety

Memory safety is one of Nirea's fundamental design principles.

Nirea uses exclusive ownership: every owned object has exactly one owner at a time.

Values of struct, array and `String` types are owned objects. Values of primitive types and enums are plain values: assigning or passing them copies them, and the ownership rules below don't apply to them.

The owner of a value is a local variable, a field of another owned value, or an element of an owned array.

## 7.1 Ownership transfer

Assignment transfers ownership rather than creating an implicit copy.

```nirea
Engine first = createEngine();
Engine second = first;

// first is no longer accessible.
```

The compiler prevents subsequent access through the previous owner.

Ownership is transferred when a local variable is used as:

- the initialiser of a variable declaration: `Engine second = first;`
- the right-hand side of an assignment to a variable, field or array element: `vehicle.engine = first;`
- the value of a `return` statement: `return first;`
- an element of an array initialiser, or the value of a switch-expression arm ([§10.1](10-control-flow.md#101-switch-expressions)) used in one of these positions.

Passing an object to a value parameter doesn't transfer ownership: the parameter receives a copy ([§7.3](#73-passing-values-and-references)).

Only a local variable, as a whole, can give its object to a new owner; value parameters count as local variables. An object owned by another object is part of that owner's memory and stays part of it: fields and array elements can't be given to a new owner. Neither can objects reached through a reference ([§9](09-references.md)), such as a reference parameter or `this`, because they belong to someone else. To get an independent object from any of them, call `copy()`:

```nirea
Engine spare = vehicle.engine;          // error: a field stays part of its owner
Engine spare = vehicle.engine.copy();   // ok
```

The compiler follows ownership transfers through all control flow. Using a variable after it has given its object away, or when it may have done so on some path, is a compile-time error. Assigning a new value to such a variable makes it usable again.

```nirea
Engine first = createEngine();
if (ready) {
    Engine second = first;
}
println(first.getPower());   // error: first may no longer own an object
```

A value produced by a call that returns a value, such as `createEngine()` or `first.copy()`, has no other owner, so it can be used in any of these positions.

## 7.2 Explicit copying

Copying must be requested explicitly.

```nirea
Engine first = createEngine();
Engine second = first.copy();
```

`copy()` creates an independent object.

`copy()` is built in for every struct, array and `String` type, and makes a deep copy: owned fields and elements are copied too. A struct can't declare its own method named `copy` or `equals` ([§6.9](06-types.md#69-equality)); type-defined copying and equality are open questions ([§18](18-open-questions.md)).

## 7.3 Passing values and references

A struct, array or `String` parameter is either a value or a reference.

A **value parameter**, written `Engine engine`, receives a copy of its argument: an object of its own in the called function's memory, owned by the function. The copy is deep, as with `copy()` ([§7.2](#72-explicit-copying)), and the caller's object is unchanged and stays usable. Any object can be passed by value: a local variable, a field, an array element, a reference or `this`. A call result that is a new object, such as `createEngine()`, is passed without copying, since nothing else owns it.

Inside the function, a value parameter is an owned local variable: it can be modified, assigned, returned, stored in a field or passed on by value. Changes to it aren't visible to the caller.

```nirea
struct Vehicle {
    Engine engine;

    void setEngine(Engine engine) {
        this.engine = engine;      // the field becomes the owner of the copy
    }
}

void main() {
    Engine engine = createEngine();
    Vehicle vehicle = Vehicle();
    vehicle.setEngine(engine);     // setEngine receives a copy
    println(engine.getPower());    // engine is still usable
}
```

This is the one deliberate exception to Nirea's rule against implicit copying ([§17](17-non-goals.md)): it saves writing `copy()` for every call, and works the same for structs, arrays and strings. To pass an object without copying it, declare the parameter as a reference.

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

A reference can refer to any object, including one that is part of another: `inspect(vehicle.engine)` and `inspect(engines[0])` refer to the field and the element where they are, inside their owners. Only passing them by value requires `.copy()`.

Through a reference, a function can read and write fields and elements and call methods, and the caller sees the changes. It can't give the object to a new owner or assign to it as a whole, because the object stays with its owner. To keep the value, the function stores a copy:

```nirea
void setEngine(Engine* engine) {
    this.engine = engine.copy();
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

Replacing an owned object transfers ownership to the replacement.

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

- A local variable, including a value parameter, ends when control leaves its block: at the end of the block, or through `return`, `break` or `continue`. Locals are destroyed in reverse order of declaration. A variable that has transferred ownership of its object doesn't destroy it.
- A field or array element ends with the object that contains it. Fields are destroyed in reverse order of declaration, and array elements from the last to the first.
- An object assigned to a variable, field or element replaces the old object, which is destroyed ([§7.5](#75-object-replacement)).
- An object that nothing owns, such as a call result used only as an argument for a reference parameter (`inspect(createEngine())`) or as a method receiver, is destroyed at the end of the statement that created it.

A runtime error ([§6.4](06-types.md#64-arithmetic-and-runtime-errors)) ends the program immediately, without destroying objects.

User-defined destructors, for structs that hold resources such as file handles, remain to be specified ([§18](18-open-questions.md)).

---

[← 6. Types](06-types.md) · [Contents](README.md) · [8. Struct encapsulation →](08-encapsulation.md)
