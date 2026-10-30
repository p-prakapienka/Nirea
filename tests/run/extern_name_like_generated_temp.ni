// A C function named like a short identifier works with generated code (§14).

extern "C" {
    int abs(int x);
}

int t1(int value) {
    return value + 1;
}

void main() {
    int l_x = abs(-3);
    println(t1(l_x));         // expect: 4
}
