// Integer overflow in a multiplication is a runtime error (§6.4).
// runtime-error: integer overflow

int multiply(int a, int b) {
    return a * b;
}

void main() {
    println(multiply(46340, 46340));    // expect: 2147395600
    println(multiply(65536, 65536));
    println("not reached");
}
