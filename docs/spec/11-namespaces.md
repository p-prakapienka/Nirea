# 11. Namespaces

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

    Audio.AudioEngine engine = Audio.AudioEngine();
    engine.initialise();
}
```

## Namespace rules

- A namespace has one name.
- Namespace names follow PascalCase.
- Namespace members can include functions, structs, interfaces, enums and constants.
- Namespace functions are accessed through the namespace name.
- Namespaces have no instances.
- Namespaces do not introduce runtime objects.
- Namespaces are independent of filesystem directories.
- Namespace declarations do not imply nested namespace hierarchies.
- Namespaces can't be nested, and each namespace is declared once in a program.
- Inside a namespace, its own members can be used without the namespace prefix. A namespace member can have the same name as a top-level declaration. Inside the namespace, the member takes precedence.
- A namespace can contain `extern "C"` blocks ([§14](14-c-interop.md)).

Namespaces replace the need for static classes and static methods.

---

[← 10. Control flow](10-control-flow.md) · [Contents](README.md) · [12. Source files and imports →](12-source-files.md)
