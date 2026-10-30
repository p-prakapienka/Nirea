// A struct can't contain itself through an array (§5.2). The struct is on one
// line so the diagnostic location is unambiguous.

struct Tree { int value; Tree[2] children; } // error

void main() {
    println("self");
}
