// Implicit widening from unsigned integer types and from float, following the
// table in §6.2.

void main() {
    uint8 byte = 200;
    uint16 byteAsUint16 = byte;
    uint byteAsUint = byte;
    uint64 byteAsUint64 = byte;
    int16 byteAsInt16 = byte;
    int byteAsInt = byte;
    int64 byteAsInt64 = byte;
    float byteAsFloat = byte;
    float64 byteAsFloat64 = byte;
    println(byteAsUint16);          // expect: 200
    println(byteAsUint);            // expect: 200
    println(byteAsUint64);          // expect: 200
    println(byteAsInt16);           // expect: 200
    println(byteAsInt);             // expect: 200
    println(byteAsInt64);           // expect: 200
    if (byteAsFloat == 200.0) {
        println("uint8 to float");          // expect: uint8 to float
    }
    if (byteAsFloat64 == 200.0) {
        println("uint8 to float64");        // expect: uint8 to float64
    }

    uint16 word = 60000;
    uint wordAsUint = word;
    uint64 wordAsUint64 = word;
    int wordAsInt = word;
    int64 wordAsInt64 = word;
    float wordAsFloat = word;
    float64 wordAsFloat64 = word;
    println(wordAsUint);            // expect: 60000
    println(wordAsUint64);          // expect: 60000
    println(wordAsInt);             // expect: 60000
    println(wordAsInt64);           // expect: 60000
    if (wordAsFloat == 60000.0) {
        println("uint16 to float");         // expect: uint16 to float
    }
    if (wordAsFloat64 == 60000.0) {
        println("uint16 to float64");       // expect: uint16 to float64
    }

    uint big = 4000000000;
    uint64 bigAsUint64 = big;
    int64 bigAsInt64 = big;
    float64 bigAsFloat64 = big;
    println(bigAsUint64);           // expect: 4000000000
    println(bigAsInt64);            // expect: 4000000000
    if (bigAsFloat64 == 4000000000.0) {
        println("uint to float64");         // expect: uint to float64
    }

    float single = 2.5;
    float64 singleAsFloat64 = single;
    println(singleAsFloat64);       // expect: 2.5
}
