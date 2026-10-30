// Floating-point literals have a fractional part, an exponent or both (§6.3).

void main() {
    float64 withFraction = 2.5;
    float64 withExponent = 25e-1;
    float64 withBoth = 0.25e1;
    println(withFraction);          // expect: 2.5
    println(withExponent);          // expect: 2.5
    println(withBoth);              // expect: 2.5

    float64 small = 2.0e-3;
    if (small == 0.002) {
        println("2.0e-3");          // expect: 2.0e-3
    }
    float64 billion = 1e9;
    if (billion == 1000000000.0) {
        println("1e9");             // expect: 1e9
    }
}
