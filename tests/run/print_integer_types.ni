// Every integer type prints in decimal, including its extreme values (§6.1, §15).

void main() {
    int8 int8Low = -127;
    int8 int8One = 1;
    int8 int8High = 127;
    println(int8Low - int8One);     // expect: -128
    println(int8High);              // expect: 127

    int16 int16Low = -32767;
    int16 int16One = 1;
    int16 int16High = 32767;
    println(int16Low - int16One);   // expect: -32768
    println(int16High);             // expect: 32767

    int intLow = -2147483647;
    int intHigh = 2147483647;
    println(intLow - 1);            // expect: -2147483648
    println(intHigh);               // expect: 2147483647

    int64 int64Low = -9223372036854775807;
    int64 int64High = 9223372036854775807;
    println(int64Low - 1);          // expect: -9223372036854775808
    println(int64High);             // expect: 9223372036854775807

    uint8 uint8Zero = 0;
    uint8 uint8High = 255;
    uint16 uint16High = 65535;
    uint uintHigh = 4294967295;
    uint64 uint64High = 18446744073709551615;
    println(uint8Zero);             // expect: 0
    println(uint8High);             // expect: 255
    println(uint16High);            // expect: 65535
    println(uintHigh);              // expect: 4294967295
    println(uint64High);            // expect: 18446744073709551615
}
