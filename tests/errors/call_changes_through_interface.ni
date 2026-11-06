// An interface method changes `this` if any implementation does, so a call
// that changes an object through an interface can't also receive it (§9.3).

interface Shape {
    void grow();
    int area();
}

struct Square : Shape {
    int side;

    void grow() {
        side = side + 1;
    }

    int area() {
        return side * side;
    }
}

void resize(Shape* target, Shape* reference) {
    target.grow();
}

int compare(Shape* first, Shape* second) {
    return first.area() - second.area();
}

void main() {
    Square square = Square();
    println(compare(square, square));
    resize(square, square); // error: this call changes square (argument 1)
}
