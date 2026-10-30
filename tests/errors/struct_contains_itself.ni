// A struct can't contain itself (§5.2). The struct is on one line so the
// diagnostic location is unambiguous.

struct Node { int value; Node next; } // error

void main() {
    println("self");
}
