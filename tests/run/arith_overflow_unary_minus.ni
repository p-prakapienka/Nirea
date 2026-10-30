// Negating the smallest int overflows (§6.4).
// runtime-error: integer overflow

int negate(int value) {
    return -value;
}

void main() {
    int smallest = -2147483647 - 1;
    println(negate(-5));            // expect: 5
    println(negate(smallest));
    println("not reached");
}
