# AGENTS.md

Guidance for AI coding agents and contributors working in this repository.

## Project

Nirea is a statically typed, memory-safe, native programming language. Its syntax is inspired by Java and its programming model by C: free functions, structs with methods, interfaces, namespaces, exclusive ownership, and no classes, inheritance or garbage collector. The compiler, `nirea`, translates Nirea to C and builds native executables with GCC or Clang.

## Current state

The project is in **Phase 2: Initial compiler** (spec §19). The compiler implements the language as specified in version 0.6 of the spec.

```text
AGENTS.md                  this file
README.md                  project overview and usage
CMakeLists.txt             build for the compiler and tests
cmake/embed.cmake          embeds the runtime sources into the compiler
docs/
└── language-design.md     language design specification (v0.6, preliminary)
runtime/
├── nirea_rt.h             runtime header included by generated C
└── nirea_rt.c             runtime: printing, strings, runtime errors
src/                       the compiler, in C11
├── main.c                 command-line interface
├── driver.c               file loading, imports (§12), invoking the C compiler
├── lexer.c                tokens
├── parser.c               syntax tree (Appendix A)
├── ast.h, types.c         syntax tree and semantic types
├── sema.c                 name resolution and type checking
├── ownership.c            ownership-transfer checking (§7)
├── codegen.c              C generation
└── common.c               memory, strings, maps, diagnostics
tests/
├── run/                   programs with expected output
├── errors/                programs that must fail to compile
├── projects/              multi-file projects
└── runner/test_runner.c   test runner used by CTest
```

### Build and test

The compiler is written in C11 and built with CMake (3.16 or later). It needs GCC or Clang, both to build it and to compile the C it generates.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build -j8 --output-on-failure
```

Run a single program with `build/nirea run path/to/File.ni`. To see the generated C, use `build/nirea build File.ni -o app --emit-c out/`.

### Tests

- `tests/run/NAME.ni`: `// expect: TEXT` comments give the expected stdout lines in order. `// exit: N` sets the expected exit status. `// runtime-error: TEXT` expects a runtime error whose message contains TEXT.
- `tests/errors/NAME.ni`: mark each line that must get a diagnostic with `// error`, or `// error: TEXT` to also match the message. Any diagnostic on an unmarked line fails the test.
- `tests/projects/NAME/`: a project with `src/main/nirea/`, plus `expected.txt` for its stdout, or an `expect-error` file if it must fail to compile.
- CMake picks up new test files when you rebuild.
- Add a test for every behaviour you add or fix. Prefer one rule per test file.

## The specification is the source of truth

- `docs/language-design.md` defines the language. Read the relevant sections before writing or changing anything about the language: docs, examples or, later, compiler code.
- Do not invent semantics for anything in §18 (Open design questions). If a task depends on an open question, say so. Either ask, or propose a resolution as an explicit spec change.
- Appendix A holds the grammar. Keep it in sync with the prose and with the parser.

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

## Compiler work

- Follow the pipeline in §13.1: lexer and parser, AST, name and type resolution, ownership and lifetime analysis, C code generation, then GCC/Clang.
- The compiler must enforce all type, ownership and safety rules before it emits C. Never rely on the C compiler to catch a Nirea rule violation.
- Keep semantics independent of the backend, so a future LLVM or native backend does not change language behaviour.
- Generated C must not depend on unspecified C behaviour. Expressions are lowered to temporaries so evaluation is left to right. Checked arithmetic goes through the `nr_*` helpers in `runtime/nirea_rt.h`.
- Generated C includes only `nirea_rt.h`, so that user `extern "C"` declarations never clash with system headers. Code that needs the C library belongs in `runtime/nirea_rt.c`.
- Diagnostics use the format `path:line:col: error: message`. Messages say what is wrong and, where possible, how to fix it.
- Owned objects that hold heap memory (strings, and structs and arrays containing them) are destroyed automatically by the generated code (§7.6). See the comment at the top of `src/codegen.c`. After changing code generation, run the suite with `NIREA_CFLAGS="-fsanitize=address,undefined" ctest --test-dir build -j8` to catch leaks, double frees and undefined behaviour.
- Keep the compiler free of warnings under `-Wall -Wextra`, and keep generated C free of warnings under `-Wall`.

## Commits

- Write clear, imperative commit messages, such as `Specify destructor execution order`.
- Keep spec changes and code changes in separate commits where practical.
