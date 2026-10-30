// A hexadecimal literal's value must fit the target type: 0x80000000 is
// 2147483648, which doesn't fit in int (§6.3).

void main() {
    int mask = 0x80000000; // error
}
