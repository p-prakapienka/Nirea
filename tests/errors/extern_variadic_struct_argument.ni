// Variadic arguments to an extern "C" function must be primitives or strings (§14).

struct Point {
    int x;
    int y;
}

extern "C" {
    int printf(String format, ...);
}

void main() {
    Point point;
    printf("%d\n", point); // error
}
