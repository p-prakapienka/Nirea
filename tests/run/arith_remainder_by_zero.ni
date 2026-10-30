// Integer remainder by zero is a runtime error (§6.4).
// runtime-error: division by zero

int remainderOf(int a, int b) {
    return a % b;
}

void main() {
    println(remainderOf(7, 3));       // expect: 1
    println(remainderOf(7, 0));
    println("not reached");
}
