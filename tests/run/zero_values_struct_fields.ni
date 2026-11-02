// Struct fields start at their zero value, including nested structs and
// arrays, when the struct is created with T(). An array field starts as an
// empty array (§5.4, §6.8).

enum Level {
    LOW,
    HIGH
}

struct Inner {
    int count;
    String name;
}

struct Outer {
    int8 small;
    uint64 big;
    float64 ratio;
    boolean flag;
    char letter;
    String label;
    Level level;
    Inner inner;
    int[] values;
    Inner[] inners;
}

void main() {
    Outer outer = Outer();
    println(outer.small);           // expect: 0
    println(outer.big);             // expect: 0
    if (outer.ratio == 0.0) {
        println("ratio zero");      // expect: ratio zero
    }
    if (!outer.flag) {
        println("flag false");      // expect: flag false
    }
    if (outer.letter == '\0') {
        println("letter zero");     // expect: letter zero
    }
    if (outer.label == "") {
        println("label empty");     // expect: label empty
    }
    println(outer.level);           // expect: LOW
    println(outer.inner.count);     // expect: 0
    if (outer.inner.name == "") {
        println("inner name empty");    // expect: inner name empty
    }
    println(outer.values.length);   // expect: 0
    println(outer.inners.length);   // expect: 0
}
