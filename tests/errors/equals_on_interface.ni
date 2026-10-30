// Interface values have no equals (§6.9).

interface Shape {
    int area();
}

boolean same(Shape* a, Shape* b) {
    return a.equals(b); // error
}

void main() {
    println(1);
}
