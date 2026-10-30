// ++ is checked for overflow like any other integer arithmetic (§6.4).
// runtime-error: integer overflow

void bumpAndPrint(int start) {
    int value = start;
    value++;
    println(value);
}

void main() {
    bumpAndPrint(2147483646);       // expect: 2147483647
    bumpAndPrint(2147483647);
    println("not reached");
}
