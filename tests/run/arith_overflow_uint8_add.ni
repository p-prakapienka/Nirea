// Unsigned overflow is a runtime error too; uint8 holds 0 to 255 (§6.1, §6.4).
// runtime-error: integer overflow

uint8 add(uint8 a, uint8 b) {
    return a + b;
}

void main() {
    println(add(200, 55));          // expect: 255
    println(add(200, 56));
    println("not reached");
}
