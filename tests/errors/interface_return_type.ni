// Return values of interface type are not supported (§5.3). The function is on
// one line so the diagnostic location is unambiguous.

interface Shape {
    int area();
}

struct Square : Shape {
    int side;

    int area() {
        return side * side;
    }
}

Shape makeShape() { Square square; return square; } // error

void main() {
    Square square;
}
