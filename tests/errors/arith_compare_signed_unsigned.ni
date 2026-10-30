// Comparison operators also need operands of the same type, so comparing int
// with uint needs a cast (§6.4).

void main() {
    int a = 5;
    uint b = 3;
    if (a < b) { // error
        println("less");
    }
}
