// Casting a floating-point value outside the target type's range is a
// runtime error (§6.2, §6.4).
// runtime-error: invalid cast

int toInt(float64 value) {
    return (int) value;
}

void main() {
    println(toInt(2000000000.0));   // expect: 2000000000
    println(toInt(3.0e9));
    println("not reached");
}
