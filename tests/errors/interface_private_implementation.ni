// An interface is implemented by public methods; a private method doesn't
// count (§5.3). The struct is on one line so the diagnostic location is
// unambiguous.

interface Shape {
    int area();
}

struct Square : Shape { int side; private int area() { return side * side; } } // error

void main() {
    Square square;
}
