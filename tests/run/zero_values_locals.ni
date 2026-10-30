// A local variable declared without an initialiser holds its type's zero
// value (§6.8).

enum Mode {
    FIRST,
    SECOND
}

void main() {
    int8 a;
    int16 b;
    int c;
    int64 d;
    uint8 e;
    uint16 f;
    uint g;
    uint64 h;
    println(a);                     // expect: 0
    println(b);                     // expect: 0
    println(c);                     // expect: 0
    println(d);                     // expect: 0
    println(e);                     // expect: 0
    println(f);                     // expect: 0
    println(g);                     // expect: 0
    println(h);                     // expect: 0

    float single;
    float64 wide;
    if (single == 0.0) {
        println("float zero");      // expect: float zero
    }
    if (wide == 0.0) {
        println("float64 zero");    // expect: float64 zero
    }
    boolean flag;
    if (!flag) {
        println("boolean false");   // expect: boolean false
    }
    char letter;
    if (letter == '\0') {
        println("char zero");       // expect: char zero
    }
    String text;
    if (text == "") {
        println("string empty");    // expect: string empty
    }
    Mode mode;
    println(mode);                  // expect: FIRST
}
