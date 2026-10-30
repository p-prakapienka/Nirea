// One operand converts to the other's type: int8 + int is an int (§6.4).

void main() {
    int8 small = 100;
    int big = 100000;
    int sum = small + big;
    println(sum);             // expect: 100100
}
