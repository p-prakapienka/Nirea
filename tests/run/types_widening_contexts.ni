// Implicit widening also applies to arguments, return values and the operands
// of a binary operator (§6.2, §6.4).

int64 toInt64(int value) {
    return value;
}

float64 half(float64 value) {
    return value / 2.0;
}

void main() {
    int count = 7;
    println(toInt64(count));        // expect: 7

    float five = 5.0;
    println(half(five));            // expect: 2.5

    int8 small = 5;
    int total = small + count;
    println(total);                 // expect: 12

    uint8 byte = 255;
    int widened = byte + count;
    println(widened);               // expect: 262

    uint large = 4000000000;
    int64 one = 1;
    int64 sum = large + one;
    println(sum);                   // expect: 4000000001
}
