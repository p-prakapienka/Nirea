// Interface values can't be compared (§6.9).

interface Shape {
    int area();
}

boolean same(Shape* a, Shape* b) {
    return a == b; // error
}

void main() {
    println(1);
}
