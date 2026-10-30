// Shifting by at least the width of the shifted type is a runtime error (§6.4).
// runtime-error: shift

int shiftLeft(int value, int amount) {
    return value << amount;
}

void main() {
    println(shiftLeft(1, 31));      // expect: -2147483648
    println(shiftLeft(1, 32));
    println("not reached");
}
