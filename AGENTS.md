# AGENTS.md

Guidance for AI coding agents and contributors working in this repository.

## Project

Nirea is a statically typed, memory-safe, native programming language. Its syntax is inspired by Java and its programming model by C: free functions, structs with methods, interfaces, namespaces, exclusive ownership, and no classes, inheritance or garbage collector. The first compiler will translate Nirea to C and build native executables with GCC or Clang.

## Current state

The project is in **Phase 1: Language definition** (spec §19). There is no compiler, build system or test suite yet. The repository holds only design documentation.

```text
AGENTS.md                  this file
README.md                  project overview
docs/
└── language-design.md     language design specification (v0.2, preliminary)
```

Update the layout above and add build/test commands here as code arrives.

## The specification is the source of truth

- `docs/language-design.md` defines the language. Read the relevant sections before writing or changing anything about the language: docs, examples or, later, compiler code.
- Do not invent semantics for anything in §18 (Open design questions). If a task depends on an open question, say so. Either ask, or propose a resolution as an explicit spec change.
- Some examples touch unresolved areas and are illustrative only. Examples: `String` in §10.1 (strings are open), `char*` in the FFI example in §14 (pointer syntax is open), and `Audio.AudioEngine engine;` without an initialiser in §11 (constructors are open).

## Changing the specification

- Check every proposal against the core principles (§2) and the non-goals (§17). Implicit behaviour, hidden operations, classes, inheritance, garbage collection and user-visible raw pointers contradict the design. Flag any such conflict instead of introducing it quietly.
- When you resolve an open question, update or remove its row in §18 and every section and example it affects.
- Keep examples consistent with the naming conventions (§4), the ownership rules (§7) and each other. If an example deliberately shows an error, say so next to it.
- Update the version and status line at the top for substantive changes.
- Keep the British spelling the spec already uses (initialise, organise, behaviour, optimisation).
- Fence Nirea code with the `nirea` info string. Use `text` for diagrams and directory trees.
- Keep changes focused: one design decision per change where practical.

## Nirea code style

These rules apply to examples now and to Nirea sources later:

- Follow the naming conventions in §4.
- Use Java/C-style braces and semicolons, with 4-space indentation.
- Use the `.ni` extension for source files. Project sources live under `src/main/nirea` (§12, §16).
- Write imports as file paths with forward slashes and an explicit `.ni` extension.

## Compiler work (future)

- Follow the pipeline in §13.1: lexer and parser, AST, name and type resolution, ownership and lifetime analysis, C code generation, then GCC/Clang.
- The compiler must enforce all type, ownership and safety rules before it emits C. Never rely on the C compiler to catch a Nirea rule violation.
- Keep semantics independent of the backend, so a future LLVM or native backend does not change language behaviour.
- The compiler's implementation language and build tooling are not chosen yet. Do not pick them as a side effect of another task. Once they are decided, record them here with the build and test commands.

## Commits

- Write clear, imperative commit messages, such as `Specify destructor execution order`.
- Keep spec changes and code changes in separate commits where practical.
