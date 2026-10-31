// Reading an element at an index outside 0 to N - 1 is a runtime error (§6.6).
// runtime-error: index out of range

int elementAt(int[]* values, int index) {
    return values[index];
}

void main() {
    int[] values = {10, 20, 30};
    println(elementAt(values, 2));  // expect: 30
    println(elementAt(values, 3));
    println("not reached");
}
