// Expressions are evaluated left to right, including function arguments and
// the operands of binary operators, whatever their precedence (§10).

int trace(int value) {
    println(value);
    return value;
}

int combine(int a, int b, int c) {
    return a * 100 + b * 10 + c;
}

void main() {
    int result = combine(trace(1), trace(2), trace(3));
    println(result);
    int sum = trace(4) + trace(5) * trace(6);
    println(sum);
    if (trace(7) < trace(8)) {
        println("compared");
    }
}

// expect: 1
// expect: 2
// expect: 3
// expect: 123
// expect: 4
// expect: 5
// expect: 6
// expect: 34
// expect: 7
// expect: 8
// expect: compared
