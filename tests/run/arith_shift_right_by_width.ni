// Right shifts are limited by the width of the shifted type too (§6.4).
// runtime-error: shift

int64 shiftRight(int64 value, int amount) {
    return value >> amount;
}

void main() {
    println(shiftRight(-8, 1));     // expect: -4
    println(shiftRight(-8, 64));
    println("not reached");
}
