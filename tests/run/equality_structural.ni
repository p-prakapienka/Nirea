// == compares structure: strings by bytes, structs by fields, arrays by elements (§6.9).

struct Point {
    int x;
    int y;
    String label;

    Point(int x, int y, String label) {
        this.x = x;
        this.y = y;
        this.label = label;
    }
}

void main() {
    String a = "abc";
    String b = "ab";
    b.appendChar('c');
    println(a == b);                     // expect: true
    println(a != "abd");                 // expect: true

    Point p = Point(1, 2, "p");
    Point q = Point(1, 2, "p");
    println(p == q);                     // expect: true
    q.label.appendChar('!');
    println(p == q);                     // expect: false

    int[3] xs = {1, 2, 3};
    int[3] ys = {1, 2, 3};
    println(xs == ys);                   // expect: true
    ys[2] = 4;
    println(xs != ys);                   // expect: true

    String[2] names = {"a", "b"};
    String[2] others = {"a", "b"};
    println(names == others);            // expect: true
}
