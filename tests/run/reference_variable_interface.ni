// A reference variable of an interface type refers to a struct that implements
// it, and calls through it use dynamic dispatch (§5.3, §9.1).

interface Shape {
    int area();
    void grow();
}

struct Square : Shape {
    int side;

    int area() {
        return side * side;
    }

    void grow() {
        side = side + 1;
    }
}

int measure(Shape* shape) {
    return shape.area();
}

void main() {
    Square square = Square();
    square.side = 3;
    Shape* shape = square;
    shape.grow();
    println(shape.area());            // expect: 16
    println(measure(shape));          // expect: 16
    Shape* other = shape;
    println(other.area());            // expect: 16
    println(square.side);             // expect: 4
}
