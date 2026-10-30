// Decimal, hexadecimal and binary integer literals, with optional
// underscores between digits (§6.3).

void main() {
    println(42);                    // expect: 42
    println(0x2A);                  // expect: 42
    println(0b101010);              // expect: 42
    println(1_000_000);             // expect: 1000000
    println(0xFF_FF);               // expect: 65535
    println(0b1111_0000);           // expect: 240
    println(0);                     // expect: 0
    int64 largest = 0x7FFF_FFFF_FFFF_FFFF;
    println(largest);               // expect: 9223372036854775807
    uint64 allOnes = 0xFFFF_FFFF_FFFF_FFFF;
    println(allOnes);               // expect: 18446744073709551615
    uint8 bits = 0b1000_0001;
    println(bits);                  // expect: 129
}
