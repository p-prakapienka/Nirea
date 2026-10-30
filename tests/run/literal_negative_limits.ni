// A negated literal counts as a single literal (§6.3).

void main() {
    int8 low = -128;
    println(low);             // expect: -128
    int64 lowest = -9223372036854775808;
    println(lowest);          // expect: -9223372036854775808
}
