// Only a struct that implements the interface can be passed for an
// interface-typed parameter (§5.3).

interface Shape {
    int area();
}

struct Label {
    String text;
}

int measure(Shape shape) {
    return shape.area();
}

void main() {
    Label label;
    int size = measure(label); // error
}
