# 12. Source files and imports

Imports reference source files, not namespaces or directories.

The project source root is `src/main/nirea`.

Source files use the `.ni` extension.

## 12.1 Project structure

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

## 12.2 Import syntax

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

## 12.3 Import rules

- Imports identify individual source files.
- Import paths use forward slashes.
- Imports can be absolute relative to the source root or relative to the importing file.
- The file extension is explicit.
- Importing a file makes its declarations available.
- Namespace access remains explicit.
- The compiler detects unresolved imports and duplicate declarations.

Importing a file does not automatically merge its namespace with the importing source file.

Resolution:

- A path starting with `./` or `../` is relative to the importing file.
- Any other path is looked up both relative to the importing file's directory and relative to the source root. If both exist and are different files, the import is ambiguous, which is an error. If neither exists, the import is unresolved.
- Imports aren't transitive: a file sees its own declarations and the declarations of the files it imports directly.
- Importing the same file again has no further effect. Circular imports are allowed.
- Every `.ni` file under the source root is part of the program, and top-level names must be unique across the whole program.
- A relative path can lead outside the source root, for example to a file shared by several projects. A file outside the source root is part of the program only when a file of the program imports it.

---

[← 11. Namespaces](11-namespaces.md) · [Contents](README.md) · [13. Native compilation →](13-compilation.md)
