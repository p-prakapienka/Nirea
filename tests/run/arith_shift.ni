// << shifts in zeros and discards the bits shifted out, without an overflow
// check. >> is arithmetic for signed types and logical for unsigned types.
// The shift amount can have any integer type (§6.4).

void main() {
    int one = 1;
    println(one << 4);              // expect: 16
    println(one << 31);             // expect: -2147483648

    int big = 0x7FFFFFFF;
    println(big << 4);              // expect: -16

    int negative = -16;
    println(negative >> 2);         // expect: -4

    int8 small = -64;
    println(small >> 3);            // expect: -8

    uint high = 0x80000000;
    println(high >> 31);            // expect: 1
    uint allOnes = 0xFFFFFFFF;
    println(allOnes >> 28);         // expect: 15

    uint8 byte = 255;
    uint8 shifted = byte << 1;
    println(shifted);               // expect: 254
    uint8 topBit = 0x80;
    println(topBit >> 7);           // expect: 1

    uint8 amount = 3;
    println(one << amount);         // expect: 8
    int64 wide = 1;
    int64 longAmount = 40;
    println(wide << longAmount);    // expect: 1099511627776
}
