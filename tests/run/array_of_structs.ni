// Arrays of structs: elements are reached by index, and assigning to an
// element moves the value in (§6.6, §7.1).

struct Point {
    int x;
    int y;

    int sum() {
        return x + y;
    }
}

void main() {
    Point[3] points;
    points[1].x = 5;
    points[1].y = 6;
    println(points[0].x);           // expect: 0
    println(points[1].sum());       // expect: 11

    Point extra;
    extra.x = 9;
    points[2] = extra;
    println(points[2].x);           // expect: 9

    points[1] = Point();
    println(points[1].sum());       // expect: 0
}
