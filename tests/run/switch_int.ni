// Integer switches have a default branch; case values can be negative or
// written in hexadecimal (§10.1, §10.2, Appendix A).

String size(int n) {
    return switch (n) {
        case -1 -> "minus one";
        case 0 -> "zero";
        case 1, 2, 3 -> "small";
        case 0x10 -> "sixteen";
        default -> "large";
    };
}

void main() {
    println(size(-1));              // expect: minus one
    println(size(0));               // expect: zero
    println(size(2));               // expect: small
    println(size(3));               // expect: small
    println(size(16));              // expect: sixteen
    println(size(99));              // expect: large
    int code = 7;
    switch (code) {
        case 7 -> println("seven"); // expect: seven
        default -> println("other");
    }
}
