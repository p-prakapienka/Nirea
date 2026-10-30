// print and println accept one value of a primitive, enum or String type (§15).

struct Point {
    int x;
    int y;
}

void main() {
    Point point;
    println(point); // error
}
