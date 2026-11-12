# 9. References

A reference, written `T*`, refers to an object where it is in memory, without owning it ([§7.3](07-ownership.md#73-passing-values-and-references)). References come in four kinds:

- reference parameters ([§7.3](07-ownership.md#73-passing-values-and-references));
- `this`, inside methods and constructors;
- reference variables ([§9.1](#91-reference-variables));
- results of functions and methods that return a reference ([§9.2](#92-returning-references)).

Only struct, array, `String` and interface types have references. A reference never owns its object, and nothing is destroyed when a reference ends. Assigning a reference to a variable or field without `*`, returning it as a value or passing it to a value parameter gives the new owner a copy of its object ([§7.1](07-ownership.md#71-assignment-copies)).

## 9.1 Reference variables

A reference variable is declared with `*` after its type, and must be initialised:

```nirea
void main() {
    Vehicle vehicle = Vehicle();
    Engine* engine = vehicle.engine;    // refers to the engine inside vehicle
    engine.setPower(150);
    engine.start();
}
```

The initialiser must be an existing object: a local variable, a parameter, `this`, a field or array element of one of these, another reference, or a call that returns a reference. A reference can't refer to a temporary object, such as a constructor call, a string literal or a call that returns a value. A reference variable of an interface type can refer to any struct value that implements the interface: `Drawable* shape = circle;`.

A reference variable is set once, when it is declared. Assigning to it as a whole is an error:

```nirea
Engine* engine = vehicle.engine;
engine.power = 200;                 // changes the engine inside vehicle
engine = other.engine;              // error: a reference is set when it is declared
```

Through a reference variable, code can read and write fields and elements and call methods, exactly as through a reference parameter ([§7.3](07-ownership.md#73-passing-values-and-references)). A reference variable is scoped like any local variable, and refers only to an object that exists when it is declared, so it can't outlive that object. [Section 9.3](#93-reading-and-changing) describes when it can be used together with other references.

## 9.2 Returning references

A function or method returns a reference when `*` follows its return type. A method that returns `this` lets calls be chained:

```nirea
struct Builder {
    int width;
    int height;

    Builder* setWidth(int width) {
        this.width = width;
        return this;
    }

    Builder* setHeight(int height) {
        this.height = height;
        return this;
    }
}

void main() {
    Builder builder = Builder();
    builder.setWidth(10).setHeight(20);     // both calls change builder
    Builder* same = builder.setWidth(30);   // a reference to builder
    Builder other = builder.setWidth(40);   // a copy of builder
}
```

A function can only return a reference to an object that outlives the call: `this`, a reference parameter, a field or array element reached through one of them, or a reference variable or call result that refers to one of these. Returning a reference to a local variable, a value parameter or a temporary object is an error, because that object is destroyed when the function returns. Functions in `extern "C"` blocks can't return references.

The caller can use the result directly, as in the chained calls above, or store it in a reference variable ([§9.1](#91-reference-variables)). Either way, the result counts as reaching the object the method was called on and every argument passed to a reference parameter, each as a whole ([§9.3](#93-reading-and-changing)). Storing the result requires all of these to be existing objects, not temporaries.

## 9.3 Reading and changing

An object can be read through any number of references at once, but it can be changed only while nothing else uses it. So an object can't be changed or replaced while a reference into it is still in use, and every reference stays valid.

Changing an object means assigning to it or to one of its parts, using `++`, `--` or a compound assignment on it, calling a method that changes `this`, passing it to a reference parameter that the function changes, or calling `append`, `appendChar` or `setCharAt` on a `String`. Anything else, such as reading its fields, calling methods that don't change `this`, copying it or comparing it, only reads it.

Nothing marks which methods change `this` or which reference parameters a function changes: the compiler works it out from the code. A method changes `this` if its body changes `this` or a part of it, directly or through a call, and a function changes a reference parameter in the same way. An interface method changes `this` if any of its implementations does. A reference variable is *changing* if its object is changed through it, and *reading* otherwise.

Two objects overlap when they are the same object or one is part of the other: `vehicle`, `vehicle.engine` and `vehicle.engine.power` all overlap, while `vehicle.engine` and `vehicle.speed` don't. Array elements with different constant indexes, such as `engines[0]` and `engines[1]`, don't overlap. An element whose index is computed at run time, such as `engines[i]`, may be any element, so it overlaps every element of the array.

A reference variable ends when an overlapping object is changed in another way: through its owner, a parameter, `this` or another reference, or by declaring a changing reference variable to it. A changing reference variable must be the only way to its object while it is in use, so it also ends when an overlapping object is used in any other way, even only to read it, or another reference variable to it is declared.

```nirea
Engine* a = vehicle.engine;          // a and b only read
Engine* b = vehicle.engine;
println(a.power + b.power);          // both usable
println(vehicle.engine.power);       // reading through the owner: still usable
vehicle.engine.power = 100;          // a change: a and b end
println(a.power);                    // error: a can no longer be used

Engine* engine = vehicle.engine;     // changing: engine.power is assigned below
vehicle.speed = 50;                  // speed doesn't overlap the engine
engine.power = 120;
println(vehicle.engine.power);       // any other use ends a changing reference
engine.power = 130;                  // error: engine can no longer be used
```

Using a reference variable, including passing it to a reference parameter, doesn't end it. Reference parameters and `this` never end: using an object through them ends the reference variables that overlap it, as above. A reference variable that has ended stays ended until the end of its scope. As with initialisation ([§6.8](06-types.md#68-zero-values-and-initialisation)), the compiler follows this through all control flow: using a reference variable that has ended, or may have ended on some path, is a compile-time error.

A call can receive the same object, or overlapping objects, more than once, as the object it is called on and as arguments for reference parameters, as long as it changes none of them. If it changes one of them, that one can't overlap the others:

```nirea
total(counter, counter);             // ok: total only reads both parameters
addFrom(counter, counter);           // error: addFrom changes its first parameter
Counter other = counter;             // a copy
addFrom(counter, other);             // ok
```

Arguments for value parameters are copies ([§7.3](07-ownership.md#73-passing-values-and-references)), so they never overlap.

Because an object can't be changed while another reference into it is in use, replacing an object can't leave a reference to the old object behind. An object moves to a new owner only when nothing uses it afterwards ([§7.2](07-ownership.md#72-moving-instead-of-copying)), so a move can't either.

### References into global objects

Any function can reach a global object by its name, so a call can reach it again while a reference into it is held, even though nothing in the calling function shows it. If that call replaces part of the global object, for example an array, the reference would be left pointing at freed memory. The compiler prevents this.

The compiler knows which global objects each function can reach: those it calls a method on by name, and those reached by every function it calls, through any chain of calls. A call through an interface can reach whatever any implementation of the method can reach.

Inside the methods of a struct type that has a global object, `this` may be that global object. There, a reference into the object's state is any reference whose object is reached through `this`: a reference variable, a part of `this` used as the object a method is called on, or a part of `this`, or `this` itself, passed to a reference parameter. Two rules apply to such references:

- A call that can reach the global object ends every reference variable into its state, as if the object had been used directly.
- A call that can reach the global object can't receive a reference into its state, as the object it is called on or as a reference argument. Calling one of the object's own methods on `this` is the exception, since that is the same way to reach it.

```nirea
struct UserStore {
    private User[] users;

    void rename(int i, String name) {
        AUDIT.log("rename");         // ok: no reference into USERS is held
        users[i].name = name;        // reaches whatever array is there now

        User* user = users[i];
        AUDIT.log("rename");         // ends user, if AUDIT.log can reach USERS
        user.name = name;            // error: user has ended

        users[i].rename(name);       // error if User.rename can reach USERS
        DATABASE.insert(users[i]);   // ok if insert takes a User value: it receives a copy
    }
}
```

Calls that can't reach the global object are unaffected. The error message shows the chain of calls through which a call can reach the global object.

---

[← 8. Struct encapsulation](08-encapsulation.md) · [Contents](README.md) · [10. Control flow →](10-control-flow.md)
