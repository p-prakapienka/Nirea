// Float-to-integer casts truncate toward zero (§6.2).

void main() {
    float64 positive = 2.9;
    float64 negative = -2.9;
    println((int) positive);        // expect: 2
    println((int) negative);        // expect: -2

    float single = 7.75;
    println((int16) single);        // expect: 7

    float64 big = 1.0e10;
    println((int64) big);           // expect: 10000000000

    float64 byteSized = 200.7;
    println((uint8) byteSized);     // expect: 200

    println((int) 3.5);             // expect: 3
    float64 whole = -3.0;
    println((int8) whole);          // expect: -3
}
