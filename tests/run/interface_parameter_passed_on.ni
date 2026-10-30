// An interface-typed parameter can be passed on to another parameter of the same interface (§5.3).

interface Shape {
    int area();
}

struct Rect : Shape {
    int width;
    int height;

    Rect(int width, int height) {
        this.width = width;
        this.height = height;
    }

    int area() {
        return width * height;
    }
}

int measure(Shape shape) {
    return shape.area();
}

int measureTwice(Shape shape) {
    return measure(shape) + measure(shape);
}

void main() {
    Rect rect = Rect(2, 3);
    println(measureTwice(rect)); // expect: 12
}
