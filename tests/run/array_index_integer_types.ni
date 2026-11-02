// An array index can have any integer type (§6.6).

void main() {
    int[] values = {10, 20, 30, 40};
    int8 tiny = 0;
    uint8 small = 1;
    int64 big = 2;
    uint16 medium = 3;
    println(values[tiny]);          // expect: 10
    println(values[small]);         // expect: 20
    println(values[big]);           // expect: 30
    println(values[medium]);        // expect: 40
}
