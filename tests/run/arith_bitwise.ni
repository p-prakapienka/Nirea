// &, |, ^ and ~ work on integer types (§6.4).

void main() {
    int a = 0x0F;
    int b = 0x3C;
    println(a & b);                 // expect: 12
    println(a | b);                 // expect: 63
    println(a ^ b);                 // expect: 51
    println(~a);                    // expect: -16
    int zero = 0;
    println(~zero);                 // expect: -1

    uint8 byte = 0;
    uint8 inverted = ~byte;
    println(inverted);              // expect: 255
    uint word = 0;
    println(~word);                 // expect: 4294967295

    uint8 mask = 0xF0;
    uint8 value = 0x3C;
    println(mask & value);          // expect: 48
    int64 wide = 0x100000000;
    int64 low = 1;
    println(wide | low);            // expect: 4294967297
}
