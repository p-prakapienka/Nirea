// Arrays can be passed by value (copied) or by reference (§6.6, §7.3).

int sumAndClear(int[3] values) {
    int total = values[0] + values[1] + values[2];
    values[0] = 0;
    return total;
}

void double(int[3]* values) {
    for (int i = 0; i < values.length; i++) {
        values[i] = values[i] * 2;
    }
}

void main() {
    int[3] values = {1, 2, 3};
    println(sumAndClear(values.copy()));  // expect: 6
    println(values[0]);                   // expect: 1
    double(values);
    println(values[2]);                   // expect: 6
    println(sumAndClear(values));         // expect: 12
}
