// A local variable can't reuse the name of a parameter in scope (§10).

int twice(int value) {
    int value = 2; // error
    return value;
}

void main() {
    println(twice(3));
}
