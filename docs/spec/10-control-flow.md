# 10. Control flow

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

`for` loops have the Java and C form: `for (int i = 0; i < count; i++) { ... }`. Conditions must have type `boolean`; numbers don't convert to `boolean`.

Expressions are evaluated left to right, including function arguments. `&&` and `||` evaluate their right operand only when needed.

Assignment (`=` and compound assignments such as `+=`), `++` and `--` are statements, not expressions, so they can't be nested inside other expressions.

A function with a non-`void` return type must return a value on every path. A path ends with `return` or with a loop that never exits: `while (true)`, `for (;;)` or `do ... while (true)` without a `break`. An `if` with an `else`, or a switch statement, returns on every path when every branch does.

Further rules:

- A local variable can't have the same name as another local variable or parameter that is in scope. A variable declared in a `for` header is scoped to the loop.
- `break` and `continue` outside a loop are errors.
- Statements after `return`, `break` or `continue` are allowed and never run.
- Only calls can be used as expression statements. A call's result may be discarded.

## 10.1 Switch expressions

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

Switch can also be used as a statement. A statement arm is a single statement or a block:

```nirea
switch (state) {
    case IDLE -> println("Idle");
    case RUNNING, STOPPED -> {
        println("Busy");
    }
}
```

An arm of a switch expression is a single expression. A `case` can list several values, separated by commas. Case values are enum values (written unqualified), integer, character, boolean or string literals. Qualified enum values (`case State.IDLE`) and named constants aren't allowed as case values. A switch compares its subject with the case values, so the subject must be a value, not a reference ([§6.9](06-types.md#69-equality)).

A switch expression of a struct, array or `String` type is used where its result gets an owner ([§7.1](07-ownership.md#71-ownership-transfer)): as an initialiser, assigned, returned or passed by value. If a branch is an existing object (a variable, field, array element or reference), the switch can only be used there; elsewhere, for example to call a method on its result, it would act on an implicit copy of that object. A switch whose branches all create new objects, such as constructor calls or string literals, can be used anywhere.

`break` and `continue` inside a switch arm apply to the enclosing loop.

## 10.2 Exhaustiveness

Every switch must be exhaustive.

- Every enum value must be covered.
- Closed sets of variants must be fully covered.
- A `default` branch can handle remaining values.
- Integer switches require a `default` branch.
- Character and string switches require a `default` branch.
- Boolean switches must cover both values unless a default exists.
- Duplicate cases are compilation errors.

There is no implicit fall-through.

Switch expressions produce values and can be used in assignments, returns and other expressions.

---

[← 9. References](09-references.md) · [Contents](README.md) · [11. Namespaces →](11-namespaces.md)
