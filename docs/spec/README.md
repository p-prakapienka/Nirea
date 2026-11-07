# Nirea — Language Design Specification

**Version:** 0.15 — Assignment copies
**Status:** Preliminary Specification

This specification defines the Nirea language exactly, one chapter per page. To learn the language by example, read the [guide](../guide/README.md) instead.

## Contents

- [1. Vision](01-vision.md)
- [2. Core principles](02-principles.md)
- [3. Language characteristics](03-characteristics.md)
- [4. Naming conventions](04-naming.md)
- [5. Basic syntax](05-syntax.md)
  - [5.1 Functions](05-syntax.md#51-functions)
  - [5.2 Structs](05-syntax.md#52-structs)
  - [5.3 Interfaces](05-syntax.md#53-interfaces)
  - [5.4 Constructors](05-syntax.md#54-constructors)
  - [5.5 Constants](05-syntax.md#55-constants)
  - [5.6 Global objects](05-syntax.md#56-global-objects)
- [6. Types](06-types.md)
  - [6.1 Primitive types](06-types.md#61-primitive-types)
  - [6.2 Conversion rules](06-types.md#62-conversion-rules)
  - [6.3 Literals](06-types.md#63-literals)
  - [6.4 Arithmetic and runtime errors](06-types.md#64-arithmetic-and-runtime-errors)
  - [6.5 Strings](06-types.md#65-strings)
  - [6.6 Arrays](06-types.md#66-arrays)
  - [6.7 Enums](06-types.md#67-enums)
  - [6.8 Zero values and initialisation](06-types.md#68-zero-values-and-initialisation)
  - [6.9 Equality](06-types.md#69-equality)
- [7. Ownership and memory safety](07-ownership.md)
  - [7.1 Assignment copies](07-ownership.md#71-assignment-copies)
  - [7.2 Moving instead of copying](07-ownership.md#72-moving-instead-of-copying)
  - [7.3 Passing values and references](07-ownership.md#73-passing-values-and-references)
  - [7.4 Nested ownership](07-ownership.md#74-nested-ownership)
  - [7.5 Object replacement](07-ownership.md#75-object-replacement)
  - [7.6 Resource lifecycle](07-ownership.md#76-resource-lifecycle)
- [8. Struct encapsulation](08-encapsulation.md)
- [9. References](09-references.md)
  - [9.1 Reference variables](09-references.md#91-reference-variables)
  - [9.2 Returning references](09-references.md#92-returning-references)
  - [9.3 Reading and changing](09-references.md#93-reading-and-changing)
- [10. Control flow](10-control-flow.md)
  - [10.1 Switch expressions](10-control-flow.md#101-switch-expressions)
  - [10.2 Exhaustiveness](10-control-flow.md#102-exhaustiveness)
- [11. Namespaces](11-namespaces.md)
- [12. Source files and imports](12-source-files.md)
  - [12.1 Project structure](12-source-files.md#121-project-structure)
  - [12.2 Import syntax](12-source-files.md#122-import-syntax)
  - [12.3 Import rules](12-source-files.md#123-import-rules)
- [13. Native compilation](13-compilation.md)
  - [13.1 Initial compiler architecture](13-compilation.md#131-initial-compiler-architecture)
  - [13.2 Implementation limits](13-compilation.md#132-implementation-limits)
  - [13.3 Why C as an intermediate language?](13-compilation.md#133-why-c-as-an-intermediate-language)
- [14. C interoperability](14-c-interop.md)
- [15. Libraries](15-libraries.md)
- [16. Project management and build structure](16-build.md)
- [17. Explicit non-goals](17-non-goals.md)
- [18. Open design questions](18-open-questions.md)
- [19. Development roadmap](19-roadmap.md)
- [20. Project identity](20-identity.md)
- [Appendix A. Grammar](grammar.md)

## Version history

- **0.3** resolved the open questions the first compiler depends on: constructors, strings, arrays, integer overflow, parameter passing and ownership transfer, interface dispatch, constants, imports and the C function interface.
- **0.4** adds value and reference parameters ([§7.3](07-ownership.md#73-passing-values-and-references)): `Engine engine` receives its own object and becomes its owner, `Engine* engine` refers to the caller's object in place.
- **0.5** makes `String` a mutable object type with built-in methods ([§6.5](06-types.md#65-strings)), frees owned objects automatically when their owner ends ([§7.6](07-ownership.md#76-resource-lifecycle)), and defines `==` as structural equality ([§6.9](06-types.md#69-equality)).
- **0.6** separates comparing values (`==`) from comparing references (`is`, removed in 0.10).
- **0.7** adds `equals` to compare the values references point to ([§6.9](06-types.md#69-equality)).
- **0.8** adds string concatenation with `+` ([§6.5](06-types.md#65-strings)).
- **0.9** makes value parameters receive copies of their arguments ([§7.3](07-ownership.md#73-passing-values-and-references)).
- **0.10** adds reference variables and functions that return references, allows only one way to reach an object at a time ([§9](09-references.md)), and removes aliases and `is`.
- **0.11** requires local variables to be assigned before they are used ([§6.8](06-types.md#68-zero-values-and-initialisation)).
- **0.12** makes arrays Java-style: the type `T[]` has no size, and an array's length is fixed when it is created with `T[n]` ([§6.6](06-types.md#66-arrays)).
- **0.13** adds global objects, struct values that belong to the program and are used through their methods ([§5.6](05-syntax.md#56-global-objects), [§9.3](09-references.md#93-reading-and-changing)).
- **0.14** lets any number of references read an object at once; only changing it needs exclusive access ([§9.3](09-references.md#93-reading-and-changing)).
- **0.15** makes assignment copy: a variable, field or parameter without `*` always holds an object of its own, `copy()` is removed, and a local variable that isn't used again moves instead of being copied ([§7.1](07-ownership.md#71-assignment-copies), [§7.2](07-ownership.md#72-moving-instead-of-copying)).

[Section 18](18-open-questions.md) lists what remains open.
