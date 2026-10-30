// Structs, enums and constants can be used before their declaration (§5.1).

void main() {
    Point p = Point(1, 2);
    println(p.x + p.y + OFFSET);  // expect: 13
    println(Color.BLUE);          // expect: BLUE
}

struct Point {
    int x;
    int y;

    Point(int x, int y) {
        this.x = x;
        this.y = y;
    }
}

const int OFFSET = 10;

enum Color { RED, BLUE }
