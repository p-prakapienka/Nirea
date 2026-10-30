// Free functions with parameters, return values and early return (§5.1).

int add(int a, int b) {
    return a + b;
}

void report(int value) {
    if (value < 0) {
        println("negative");
        return;
    }
    println("not negative");
}

void main() {
    int result = add(10, 20);
    println(result);                // expect: 30
    println(add(add(1, 2), 3));     // expect: 6
    report(-4);                     // expect: negative
    report(4);                      // expect: not negative
}
