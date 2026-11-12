# 17. Explicit non-goals

The initial language deliberately excludes:

- Mandatory classes.
- Abstract classes.
- Class and struct inheritance.
- Garbage collection.
- User-visible raw pointers.
- Implicitly shared objects: without `*`, every variable, field, array element and parameter holds an object of its own ([§7.1](07-ownership.md#71-assignment-copies)).
- Implicit narrowing conversions.
- Property syntactic sugar.
- Implicit getter/setter generation.
- Implicit fall-through.
- Mandatory virtual machines.
- Complex runtime reflection.
- Mandatory large standard libraries.
- Hidden operations and implicit conversions.

The goal is not to maximise language features, but to make a small set of features work consistently.

---

[← 16. Project management and build structure](16-build.md) · [Contents](README.md) · [18. Open design questions →](18-open-questions.md)
