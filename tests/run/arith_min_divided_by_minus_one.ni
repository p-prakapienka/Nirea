// Dividing the smallest signed value by -1 overflows (§6.4).

void main() {
    int smallest = -2147483648;
    int minusOne = -1;
    println(smallest / minusOne);
}
// runtime-error: integer overflow
