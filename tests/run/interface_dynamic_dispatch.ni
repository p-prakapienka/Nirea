// Calls through an interface-typed parameter use dynamic dispatch; any struct
// that implements the interface can be passed for it (§5.3).

interface Shape {
    int area();
    String name();
}

struct Square : Shape {
    int side;

    int area() {
        return side * side;
    }

    String name() {
        return "square";
    }
}

struct Rectangle : Shape {
    int width;
    int height;

    int area() {
        return width * height;
    }

    String name() {
        return "rectangle";
    }
}

void describe(Shape* shape) {
    print(shape.name());
    print(" ");
    println(shape.area());
}

void main() {
    Square square = Square();
    square.side = 3;
    Rectangle rectangle = Rectangle();
    rectangle.width = 2;
    rectangle.height = 5;
    describe(square);               // expect: square 9
    describe(rectangle);            // expect: rectangle 10
    square.side = 4;
    describe(square);               // expect: square 16
}
