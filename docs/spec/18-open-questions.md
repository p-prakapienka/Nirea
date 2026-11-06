# 18. Open design questions

The following decisions require further specification.

| Area        | Question                                                                                                                                    |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| References  | Whether read-only references are needed once types can own heap resources, and whether primitive values can be passed by reference (`int*`) |
| Lifetimes   | How references stored inside structs would be checked, if they are allowed                                                                  |
| Copying     | Type-defined copy and equality behaviour                                                                                                    |
| Destruction | User-defined destructors for structs that hold resources such as file handles                                                               |
| Mutability  | How is object mutation distinguished from field replacement?                                                                                |
| Overloading | Whether functions, methods and constructors can be overloaded                                                                               |
| Generics    | Whether generics are included initially                                                                                                     |
| Enums       | Variants with associated data                                                                                                               |
| Strings     | Text processing beyond the built-in `String` methods, and Unicode text                                                                      |
| Arrays      | Growable arrays (lists) and slices                                                                                                          |
| FFI         | Struct, array, pointer and callback types at the C boundary                                                                                 |
| Modules     | Per-file private declarations, and reopening a namespace in several places                                                                  |
| ABI         | Native calling conventions and data layouts                                                                                                 |
| Errors      | Recoverable error handling                                                                                                                  |
| Concurrency | Ownership and thread-safety rules, including access to global objects ([§5.6](05-syntax.md#56-global-objects))                                                               |
| Interfaces  | Owned interface values (variables, fields, return values) and static dispatch                                                               |

---

[← 17. Explicit non-goals](17-non-goals.md) · [Contents](README.md) · [19. Development roadmap →](19-roadmap.md)
