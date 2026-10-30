// An implementing method has the same name, parameter types and return type
// as the interface method (§5.3). The struct is on one line so the diagnostic
// location is unambiguous.

interface Shape {
    int area();
}

struct Square : Shape { int side; int64 area() { return 1; } } // error

void main() {
    Square square;
}
