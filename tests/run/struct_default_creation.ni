// A struct that declares no constructor can be created with T(), which gives
// a zero-initialised value (§5.4).

struct Point {
    int x;
    int y;
}

void main() {
    Point a = Point();
    println(a.x);                   // expect: 0
    a.x = 4;
    println(a.x);                   // expect: 4
    a = Point();
    println(a.x);                   // expect: 0
    println(Point().y);             // expect: 0
}
