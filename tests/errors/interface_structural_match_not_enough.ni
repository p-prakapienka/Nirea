// Matching methods don't implement an interface; it must be listed after ':' (§5.3).

interface Shape {
    int area();
}

struct Square {
    int side;

    int area() {
        return side * side;
    }
}

int measure(Shape shape) {
    return shape.area();
}

void main() {
    Square square;
    println(measure(square)); // error
}
