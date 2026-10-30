// Implicit widening from signed integer types, following the table in §6.2.

void main() {
    int8 small = -3;
    int16 smallAsInt16 = small;
    int smallAsInt = small;
    int64 smallAsInt64 = small;
    float smallAsFloat = small;
    float64 smallAsFloat64 = small;
    println(smallAsInt16);          // expect: -3
    println(smallAsInt);            // expect: -3
    println(smallAsInt64);          // expect: -3
    println(smallAsFloat);          // expect: -3.0
    println(smallAsFloat64);        // expect: -3.0

    int16 medium = -30000;
    int mediumAsInt = medium;
    int64 mediumAsInt64 = medium;
    float mediumAsFloat = medium;
    float64 mediumAsFloat64 = medium;
    println(mediumAsInt);           // expect: -30000
    println(mediumAsInt64);         // expect: -30000
    if (mediumAsFloat == -30000.0) {
        println("int16 to float");          // expect: int16 to float
    }
    if (mediumAsFloat64 == -30000.0) {
        println("int16 to float64");        // expect: int16 to float64
    }

    int large = 2000000000;
    int64 largeAsInt64 = large;
    float64 largeAsFloat64 = large;
    println(largeAsInt64);          // expect: 2000000000
    if (largeAsFloat64 == 2000000000.0) {
        println("int to float64");          // expect: int to float64
    }
}
