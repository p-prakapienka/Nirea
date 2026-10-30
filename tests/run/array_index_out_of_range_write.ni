// Writing to an element outside 0 to N - 1 is a runtime error (§6.6).
// runtime-error: index out of range

void store(int[3]* values, int index, int value) {
    values[index] = value;
}

void main() {
    int[3] values = int[3]();
    store(values, 2, 5);
    println(values[2]);             // expect: 5
    store(values, 3, 5);
    println("not reached");
}
