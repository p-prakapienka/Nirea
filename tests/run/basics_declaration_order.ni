// Functions can call each other regardless of declaration order (§5.1).

void main() {
    println(twice(21));             // expect: 42
    if (isEven(10)) {
        println("10 is even");      // expect: 10 is even
    }
    if (isOdd(7)) {
        println("7 is odd");        // expect: 7 is odd
    }
}

int twice(int value) {
    return identity(value) * 2;
}

int identity(int value) {
    return value;
}

boolean isEven(int n) {
    if (n == 0) {
        return true;
    }
    return isOdd(n - 1);
}

boolean isOdd(int n) {
    if (n == 0) {
        return false;
    }
    return isEven(n - 1);
}
