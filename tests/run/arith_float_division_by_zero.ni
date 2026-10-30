// Floating-point division by zero follows IEEE 754, producing an infinity or
// NaN, and isn't a runtime error (§6.4).

void main() {
    float64 zero = 0.0;
    float64 one = 1.0;
    float64 positive = one / zero;
    if (positive > 1.0e308) {
        println("positive infinity");       // expect: positive infinity
    }
    float64 negative = -one / zero;
    if (negative < -1.0e308) {
        println("negative infinity");       // expect: negative infinity
    }
    float64 notANumber = zero / zero;
    if (notANumber != notANumber) {
        println("not a number");            // expect: not a number
    }
    float singleZero = 0.0;
    float singleOne = 1.0;
    float singleInfinity = singleOne / singleZero;
    float64 widened = singleInfinity;
    if (widened > 1.0e308) {
        println("float infinity");          // expect: float infinity
    }
    println("no trap");                     // expect: no trap
}
