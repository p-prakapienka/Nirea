// A negative value is out of range for an unsigned target type (§6.2, §6.4).
// runtime-error: invalid cast

uint8 toByte(float64 value) {
    return (uint8) value;
}

void main() {
    println(toByte(255.0));         // expect: 255
    println(toByte(-1.0));
    println("not reached");
}
