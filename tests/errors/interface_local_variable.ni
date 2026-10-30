// Interface types can be used only as references (Shape*); owned variables of
// interface type are not supported (§5.3).

interface Shape {
    int area();
}

struct Square : Shape {
    int side;

    int area() {
        return side * side;
    }
}

void main() {
    Square square;
    Shape shape; // error
}
