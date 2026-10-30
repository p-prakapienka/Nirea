// Integer arithmetic operators and their precedence (§6.4, Appendix A).

void main() {
    int a = 17;
    int b = 5;
    println(a + b);                 // expect: 22
    println(a - b);                 // expect: 12
    println(b - a);                 // expect: -12
    println(a * b);                 // expect: 85
    println(a / b);                 // expect: 3
    println(a % b);                 // expect: 2
    println(-a);                    // expect: -17

    println(2 + 3 * 4);             // expect: 14
    println((2 + 3) * 4);           // expect: 20
    println(10 - 3 - 2);            // expect: 5
    println(100 / 10 / 2);          // expect: 5
    println(1 << 2 + 1);            // expect: 8

    uint big = 4000000000;
    uint rest = 294967295;
    println(big + rest);            // expect: 4294967295

    int64 wide = 3000000000;
    int64 three = 3;
    println(wide * three);          // expect: 9000000000
}
