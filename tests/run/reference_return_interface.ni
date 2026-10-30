// A method can return a reference to an interface, such as `this` converted to
// an interface the struct implements (§5.3, §9.2).

interface Shape {
    int area();
    Shape* self();
}

struct Square : Shape {
    int side;

    int area() {
        return side * side;
    }

    Shape* self() {
        return this;
    }
}

Shape* larger(Shape* a, Shape* b) {
    if (a.area() >= b.area()) {
        return a;
    }
    return b;
}

void main() {
    Square small;
    small.side = 2;
    Square big;
    big.side = 5;
    println(larger(small, big).area());       // expect: 25
    Shape* shape = small.self();
    println(shape.self().area());             // expect: 4
}
