// Integer-to-integer casts keep the low bits of the two's-complement
// representation (§6.2).

void main() {
    println((uint8) 300);           // expect: 44
    println((int8) 200);            // expect: -56
    println((int16) 70000);         // expect: 4464
    println((uint16) -1);           // expect: 65535
    println((uint) -1);             // expect: 4294967295
    println((uint64) -1);           // expect: 18446744073709551615

    int64 big = 3000000000;
    println((int) big);             // expect: -1294967296

    uint64 huge = 18446744073709551615;
    println((int64) huge);          // expect: -1

    int value = 1000;
    println((uint8) value);         // expect: 232
    int negative = -1;
    println((uint8) negative);      // expect: 255
    int8 small = -2;
    println((uint) small);          // expect: 4294967294
    uint8 byte = 255;
    println((int8) byte);           // expect: -1
}
