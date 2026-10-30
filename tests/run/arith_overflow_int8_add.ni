// Overflow is checked at the width of the operand type: int8 holds -128 to
// 127, so 100 + 28 overflows even though it fits in int (§6.1, §6.4).
// runtime-error: integer overflow

int8 add(int8 a, int8 b) {
    return a + b;
}

void main() {
    println(add(100, 27));          // expect: 127
    println(add(100, 28));
    println("not reached");
}
