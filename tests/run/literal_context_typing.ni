// Numeric literals take their type from context: an integer literal can be
// used as any integer or floating-point type that represents its value
// exactly (§6.3).

void takeByte(uint8 value) {
    println(value);
}

uint16 largestWord() {
    return 65535;
}

void main() {
    uint8 byte = 255;
    println(byte);                  // expect: 255
    int64 big = 3000000000;
    println(big);                   // expect: 3000000000
    uint large = 4000000000;
    println(large);                 // expect: 4000000000
    int8 negative = -100;
    println(negative);              // expect: -100
    takeByte(200);                  // expect: 200
    println(largestWord());         // expect: 65535

    float64 whole = 1;
    println(whole);                 // expect: 1.0
    float exact = 16777216;
    float64 exactWidened = exact;
    if (exactWidened == 16777216.0) {
        println("exact in float");  // expect: exact in float
    }
    float single = 2.5;
    println(single);                // expect: 2.5
}
