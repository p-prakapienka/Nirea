// println and C printf share standard output, so output keeps program order (§15).

extern "C" {
    int printf(String* format, ...);
}

void main() {
    println("first");         // expect: first
    printf("second\n");       // expect: second
    println("third");         // expect: third
}
