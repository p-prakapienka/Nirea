// Plain notation from 0.001 up to 10000000, exponent notation outside (§15).

void main() {
    println(123.25);          // expect: 123.25
    println(0.001);           // expect: 0.001
    println(0.0001);          // expect: 1.0E-4
    println(9999999.0);       // expect: 9999999.0
    println(10000000.0);      // expect: 1.0E7
    println(0.000015);        // expect: 1.5E-5
    println(-2.5);            // expect: -2.5
    float small = 0.25;
    println(small);           // expect: 0.25
}
