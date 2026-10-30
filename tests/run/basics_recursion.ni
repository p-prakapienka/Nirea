// Recursion is allowed (§5.1).

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n < 2) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int64 factorial64(int n) {
    if (n <= 1) {
        return 1;
    }
    return factorial64(n - 1) * n;
}

void main() {
    println(factorial(10));         // expect: 3628800
    println(fibonacci(15));         // expect: 610
    println(factorial64(20));       // expect: 2432902008176640000
}
