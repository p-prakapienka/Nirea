// Shifting by a negative amount is a runtime error (§6.4).
// runtime-error: shift

int shiftLeft(int value, int amount) {
    return value << amount;
}

void main() {
    println(shiftLeft(1, 3));       // expect: 8
    println(shiftLeft(1, -1));
    println("not reached");
}
