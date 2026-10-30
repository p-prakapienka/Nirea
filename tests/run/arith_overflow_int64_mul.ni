// int64 overflow is a runtime error (§6.4).
// runtime-error: integer overflow

int64 multiply(int64 a, int64 b) {
    return a * b;
}

void main() {
    println(multiply(4611686018427387903, 2));  // expect: 9223372036854775806
    println(multiply(4611686018427387904, 2));
    println("not reached");
}
