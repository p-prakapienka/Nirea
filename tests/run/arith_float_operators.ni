// Floating-point arithmetic; % gives a remainder with the sign of the
// dividend, like C's fmod (§6.4).

void main() {
    float64 a = 1.5;
    float64 b = 1.0;
    println(a + b);                 // expect: 2.5
    float64 five = 5.0;
    float64 two = 2.0;
    println(five / two);            // expect: 2.5
    float64 minusOneAndHalf = -1.5;
    println(minusOneAndHalf * two); // expect: -3.0

    float half = 0.5;
    float singleFive = 5.0;
    println(half * singleFive);     // expect: 2.5

    float64 dividend = 5.5;
    float64 divisor = 3.0;
    println(dividend % divisor);    // expect: 2.5
    float64 negativeDividend = -5.5;
    println(negativeDividend % divisor);    // expect: -2.5
    float64 negativeDivisor = -3.0;
    println(dividend % negativeDivisor);    // expect: 2.5
    float singleDividend = 6.5;
    float singleDivisor = 4.0;
    println(singleDividend % singleDivisor);    // expect: 2.5
}
