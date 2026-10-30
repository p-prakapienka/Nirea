// Casting NaN to an integer type is a runtime error (§6.2, §6.4).
// runtime-error: invalid cast

int toInt(float64 value) {
    return (int) value;
}

void main() {
    float64 zero = 0.0;
    println(toInt(2.5));            // expect: 2
    println(toInt(zero / zero));
    println("not reached");
}
