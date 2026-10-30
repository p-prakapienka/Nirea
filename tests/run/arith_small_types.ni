// Arithmetic on two operands of the same type produces a value of that type,
// so small types are not promoted to int (§6.4).

void main() {
    int8 a = 100;
    int8 b = 27;
    int8 sum = a + b;
    println(sum);                   // expect: 127

    uint8 c = 200;
    uint8 d = 55;
    uint8 total = c + d;
    println(total);                 // expect: 255

    int16 e = -300;
    int16 f = 100;
    int16 product = e * f;
    println(product);               // expect: -30000

    uint16 g = 65000;
    uint16 h = 535;
    println(g + h);                 // expect: 65535
}
