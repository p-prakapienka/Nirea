# 17. Explicit non-goals

The initial language deliberately excludes:

- Mandatory classes.
- Abstract classes.
- Class and struct inheritance.
- Garbage collection.
- User-visible raw pointers.
- Implicit copying, except for value parameters ([§7.3](07-ownership.md#73-passing-values-and-references)).
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
