// A struct that declares no constructor can be created with T(), or with a
// declaration without an initialiser; both give a zero-initialised value (§5.4).

struct Point {
    int x;
    int y;
}

void main() {
    Point a = Point();
    Point b;
    println(a.x);                   // expect: 0
    println(b.y);                   // expect: 0
    b.x = 4;
    println(b.x);                   // expect: 4
    b = Point();
    println(b.x);                   // expect: 0
    println(Point().y);             // expect: 0
}
