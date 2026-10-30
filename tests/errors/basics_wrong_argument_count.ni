// A call passes exactly one argument for each parameter (§5.1).

int add(int a, int b) {
    return a + b;
}

void main() {
    int sum = add(1); // error
}
