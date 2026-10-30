// Floating-point values print in their shortest round-trip form, always with
// a decimal point or an exponent (§15).

void main() {
    float64 one = 1.0;
    float64 tenth = 0.1;
    float64 twoAndHalf = 2.5;
    float64 minusThree = -3.0;
    println(one);                   // expect: 1.0
    println(tenth);                 // expect: 0.1
    println(twoAndHalf);            // expect: 2.5
    println(minusThree);            // expect: -3.0

    float singleOne = 1.0;
    float singleTwoAndHalf = 2.5;
    float singleMinusThree = -3.0;
    println(singleOne);             // expect: 1.0
    println(singleTwoAndHalf);      // expect: 2.5
    println(singleMinusThree);      // expect: -3.0

    float64 fromInteger = 1;
    println(fromInteger);           // expect: 1.0
    println(2.5);                   // expect: 2.5
    print(0.1);
    println(" printed");            // expect: 0.1 printed
}
