// A negative index is out of range (§6.6).
// runtime-error: index out of range

int elementAt(int[3] values, int index) {
    return values[index];
}

void main() {
    int[3] values = {10, 20, 30};
    println(elementAt(values, 0));  // expect: 10
    println(elementAt(values, -1));
    println("not reached");
}
