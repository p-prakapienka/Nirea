// An array's length is chosen when it is created, from any integer expression,
// and its elements start at their zero value (§6.6).

int[] squares(int count) {
    int[] result = int[count];
    for (int i = 0; i < result.length; i++) {
        result[i] = i * i;
    }
    return result;
}

void main() {
    int[] values = squares(5);
    println(values.length);          // expect: 5
    println(values[4]);              // expect: 16
    uint8 small = 2;
    String[] names = String[small + 1];
    println(names.length);           // expect: 3
    println(names[2].length());      // expect: 0
    int[] none = int[0];
    println(none.length);            // expect: 0
}
