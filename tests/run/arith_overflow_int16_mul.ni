// Overflow is checked at the width of the operand type: int16 holds -32768
// to 32767 (§6.1, §6.4).
// runtime-error: integer overflow

int16 multiply(int16 a, int16 b) {
    return a * b;
}

void main() {
    println(multiply(181, 181));    // expect: 32761
    println(multiply(300, 300));
    println("not reached");
}
