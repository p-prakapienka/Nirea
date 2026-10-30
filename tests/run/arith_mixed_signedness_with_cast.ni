// Mixing signed and unsigned operands is allowed once an explicit cast makes
// the types match (§6.4).

void main() {
    int a = -5;
    uint b = 3;
    println(a + (int) b);           // expect: -2
    println((uint) a + b);          // expect: 4294967294
    if (a < (int) b) {
        println("less");            // expect: less
    }

    int64 c = 10;
    uint64 d = 20;
    println(c + (int64) d);         // expect: 30

    uint8 e = 200;
    int8 f = -100;
    println((int) e + (int) f);     // expect: 100
}
