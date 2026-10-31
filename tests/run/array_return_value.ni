// Arrays can be returned from functions; returning a local transfers ownership (§6.6, §7.1).

int[] squares() {
    int[] result = int[3];
    for (int i = 0; i < result.length; i++) {
        result[i] = (i + 1) * (i + 1);
    }
    return result;
}

void main() {
    int[] values = squares();
    println(values[0]);             // expect: 1
    println(values[2]);             // expect: 9
    println(squares()[1]);          // expect: 4
    values[1] = 0;
    values = squares();
    println(values[1]);             // expect: 4
}
