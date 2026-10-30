// The range check for float-to-integer casts applies after truncation (§6.2).

void main() {
    float64 value = -0.5;
    println((uint8) value);   // expect: 0
    float64 almost = 255.9;
    println((uint8) almost);  // expect: 255
    float64 below = -1.0;
    println((uint8) below);
}
// runtime-error: invalid cast
