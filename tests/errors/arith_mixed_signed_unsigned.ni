// Mixing signed and unsigned operands of the same width needs a cast (§6.4).

void main() {
    int a = 5;
    uint b = 3;
    int sum = a + b; // error
}
