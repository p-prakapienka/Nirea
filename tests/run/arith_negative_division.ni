// Integer division truncates toward zero; the remainder has the sign of the dividend (§6.4).

void main() {
    println(-7 / 2);          // expect: -3
    println(-7 % 2);          // expect: -1
    println(7 % -2);          // expect: 1
    int smallest = -2147483648;
    println(smallest % -1);   // expect: 0
}
