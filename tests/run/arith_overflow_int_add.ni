// Integer overflow is a runtime error (§6.4).
// runtime-error: integer overflow

int add(int a, int b) {
    return a + b;
}

void main() {
    println(add(2147483646, 1));    // expect: 2147483647
    println(add(2147483647, 1));
    println("not reached");
}
