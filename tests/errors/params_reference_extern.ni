// C functions can't take references (§7.3, §14).

struct Point {
    int x;
}

extern "C" {
    void fill(Point* point); // error
}

void main() {
    println(1);
}
