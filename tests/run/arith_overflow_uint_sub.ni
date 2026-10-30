// Subtracting below zero in an unsigned type is integer overflow (§6.4).
// runtime-error: integer overflow

uint subtract(uint a, uint b) {
    return a - b;
}

void main() {
    println(subtract(5, 5));        // expect: 0
    println(subtract(0, 1));
    println("not reached");
}
