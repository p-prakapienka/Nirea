// Parameters of extern "C" functions can be primitive types and String only (§14).

struct Point {
    int x;
    int y;
}

extern "C" {
    void draw(Point point); // error
}

void main() {
    println("extern");
}
