// Compound assignments, ++ and -- as statements (§10, Appendix A).

void main() {
    int x = 10;
    x += 5;
    println(x);                     // expect: 15
    x -= 3;
    println(x);                     // expect: 12
    x *= 2;
    println(x);                     // expect: 24
    x /= 5;
    println(x);                     // expect: 4
    x %= 3;
    println(x);                     // expect: 1
    x <<= 4;
    println(x);                     // expect: 16
    x >>= 2;
    println(x);                     // expect: 4
    x |= 3;
    println(x);                     // expect: 7
    x &= 5;
    println(x);                     // expect: 5
    x ^= 1;
    println(x);                     // expect: 4
    x++;
    println(x);                     // expect: 5
    x--;
    x--;
    println(x);                     // expect: 3
    ++x;
    println(x);                     // expect: 4
    --x;
    println(x);                     // expect: 3
}
