// A function with a non-void return type must return a value on every path
// (§10). The function is on one line so the diagnostic location is unambiguous.

int sign(int value) { if (value > 0) { return 1; } else if (value < 0) { return -1; } } // error

void main() {
    println(sign(5));
}
