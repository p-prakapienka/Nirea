// Floating-point division and remainder by zero follow IEEE 754 and print as NaN or Infinity (§6.4, §15).

void main() {
    float64 zero = 0.0;
    println(1.0 / zero);      // expect: Infinity
    println(-1.0 / zero);     // expect: -Infinity
    println(1.0 % zero);      // expect: NaN
    println(zero / zero);     // expect: NaN
}
