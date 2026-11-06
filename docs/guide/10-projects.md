# 10. Namespaces and projects

## Namespaces

A namespace groups related declarations under one name, like a Java class with only static members. It has no instances and creates nothing at run time.

```nirea
namespace Geometry {
    const int SIDES = 4;

    struct Point {
        int x;
        int y;
    }

    Point point(int x, int y) {
        Point p = Point();
        p.x = x;
        p.y = y;
        return p;
    }

    int distance(Point* a, Point* b) {       // inside the namespace, no prefix is needed
        return abs(a.x - b.x) + abs(a.y - b.y);
    }

    int abs(int value) {
        if (value < 0) {
            return -value;
        }
        return value;
    }
}

void main() {
    Geometry.Point a = Geometry.point(1, 2);
    Geometry.Point b = Geometry.point(4, 6);
    println(Geometry.distance(a, b));        // 7
    println(Geometry.SIDES);                 // 4
}
```

- A namespace has one name, in PascalCase. Namespaces can't be nested, and each is declared once in a program.
- It can hold functions, structs, interfaces, enums, constants, global objects and `extern "C"` blocks.
- Outside the namespace, its members are always written with its name: `Geometry.Point`.

## Projects

A project keeps its sources under `src/main/nirea`, the source root, as in Maven, and every `.ni` file there is part of the program. As in Java, files are named relative to the source root, without the `src/main/nirea` prefix: `plants/Plant.ni`. Directories only organise the files: namespaces group declarations, and imports name the files a file uses.

```text
garden/
├── src/
│   └── main/
│       └── nirea/
│           ├── Main.ni
│           ├── plants/
│           │   └── Plant.ni
│           └── util/
│               └── Text.ni
└── target/                  created by nirea build
    ├── generated-c/         the generated C
    └── nirea-app            the executable (nirea-app.exe on Windows)
```

`util/Text.ni` declares a namespace:

```nirea
namespace Text {
    String centimetres(int value) {
        return String.of(value) + " cm";
    }
}
```

`plants/Plant.ni` imports it with a path relative to itself:

```nirea
import ../util/Text.ni;

namespace Plants {
    struct Plant {
        String name;
        int height;

        Plant(String name, int height) {
            this.name = name;
            this.height = height;
        }

        String describe() {
            return name + ", " + Text.centimetres(height);
        }
    }
}
```

`Main.ni` imports `Plant.ni` with a path from the source root:

```nirea
import plants/Plant.ni;

void main() {
    Plants.Plant[] plants = {Plants.Plant("Rose", 40), Plants.Plant("Fern", 25)};
    for (int i = 0; i < plants.length; i++) {
        println(plants[i].describe());
    }
}
```

```sh
nirea build garden           # writes garden/target/nirea-app
garden/target/nirea-app      # prints "Rose, 40 cm" and "Fern, 25 cm"
cd garden && nirea run       # the default project is the current directory
```

- Imports are file paths with forward slashes and the `.ni` extension. Paths starting with `./` or `../` are relative to the importing file; other paths are looked up next to the importing file and from the source root.
- Imports aren't transitive. `Main.ni` can use `Plants`, but not `Text`, which only `Plant.ni` imports.
- Namespaces don't depend on directories, and their names are always written out: `Plants.Plant`, `Text.centimetres`.
- Top-level names must be unique across the whole project.

---

[← 9. Global objects](09-global-objects.md) · [Contents](README.md) · [11. Calling C →](11-c-interop.md)
