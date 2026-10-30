// Interface parameters must be references (§5.3, §7.3).

interface Shape {
    int area();
}

int measure(Shape shape) { // error
    return 0;
}

void main() {
    println(1);
}
