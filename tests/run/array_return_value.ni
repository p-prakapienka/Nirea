// Arrays can be returned from functions; returning a local moves it (§6.6, §7.1).

int[3] squares() {
    int[3] result;
    for (int i = 0; i < result.length; i++) {
        result[i] = (i + 1) * (i + 1);
    }
    return result;
}

void main() {
    int[3] values = squares();
    println(values[0]);             // expect: 1
    println(values[2]);             // expect: 9
    println(squares()[1]);          // expect: 4
    values[1] = 0;
    values = squares();
    println(values[1]);             // expect: 4
}
