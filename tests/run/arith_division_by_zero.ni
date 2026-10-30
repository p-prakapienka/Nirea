// Integer division by zero is a runtime error (§6.4).
// runtime-error: division by zero

int divide(int a, int b) {
    return a / b;
}

void main() {
    println(divide(7, 2));          // expect: 3
    println(divide(7, 0));
    println("not reached");
}
