// Casts to floating-point types round to the nearest representable value (§6.2).

void main() {
    int minusThree = -3;
    println((float) minusThree);    // expect: -3.0

    int64 one = 1;
    println((float64) one);         // expect: 1.0

    uint64 count = 1;
    float asFloat = (float) count;
    println(asFloat);               // expect: 1.0

    int notExact = 33554435;
    float rounded = (float) notExact;
    float64 widened = rounded;
    if (widened == 33554436.0) {
        println("rounded to nearest");      // expect: rounded to nearest
    }

    float64 precise = 0.1;
    float narrowed = (float) precise;
    float direct = 0.1;
    if (narrowed == direct) {
        println("float64 to float");        // expect: float64 to float
    }
}
