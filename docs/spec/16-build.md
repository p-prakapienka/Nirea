# 16. Project management and build structure

Nirea adopts Maven-inspired project organisation.

The initial project convention uses:

- `src/main/nirea` for production source files.
- `src/main/resources` for production resources.
- `src/test/nirea` for test source files.
- `src/test/resources` for test resources.
- `target` for generated files and build output.
- `pom.xml` for project configuration.

Maven-style project coordinates and dependency management are intended to provide a familiar development experience.

Whether Nirea will use Maven directly or introduce a dedicated build tool remains open. The first compiler builds a project from `src/main/nirea` into `target/`, and doesn't read `pom.xml` yet.

---

[← 15. Libraries](15-libraries.md) · [Contents](README.md) · [17. Explicit non-goals →](17-non-goals.md)
