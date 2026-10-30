// A struct must declare a public method for every method of each interface it
// implements (§5.3). The struct is on one line so the diagnostic location is
// unambiguous.

interface Shape {
    int area();
    int perimeter();
}

struct Square : Shape { int side; int area() { return side * side; } } // error

void main() {
    Square square;
}
