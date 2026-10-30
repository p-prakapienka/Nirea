// The width that limits a shift is that of the shifted type: 8 for uint8 (§6.4).
// runtime-error: shift

uint8 shiftLeft(uint8 value, int amount) {
    return value << amount;
}

void main() {
    println(shiftLeft(1, 7));       // expect: 128
    println(shiftLeft(1, 8));
    println("not reached");
}
