// Functions can't be overloaded: each name is declared once in its scope (§5.1).

int scale(int value) {
    return value * 2;
}

int scale(int value, int factor) { // error
    return value * factor;
}

void main() {
    println("overloading");
}
