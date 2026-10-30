// `a is b` checks whether two references point to the same object (§6.10).

struct Engine {
    int power;

    Engine(int power) {
        this.power = power;
    }

    boolean isSelf(Engine* other) {
        return other is this;
    }
}

interface Shape {
    int area();
}

struct Square : Shape {
    int side;

    int area() {
        return side * side;
    }
}

boolean sameEngine(Engine* a, Engine* b) {
    return a is b;
}

boolean sameShape(Shape* a, Shape* b) {
    return a is b;
}

boolean sameValue(Engine* a, Engine* b) {
    return a.power == b.power;
}

void main() {
    Engine first = Engine(100);
    Engine second = Engine(100);
    println(sameEngine(first, first));    // expect: true
    println(sameEngine(first, second));   // expect: false
    println(sameValue(first, second));    // expect: true
    println(first.isSelf(first));         // expect: true
    println(first.isSelf(second));        // expect: false

    Engine[2] engines;
    println(sameEngine(engines[0], engines[0])); // expect: true
    println(sameEngine(engines[0], engines[1])); // expect: false

    Square a;
    Square b;
    println(sameShape(a, a));             // expect: true
    println(sameShape(a, b));             // expect: false
}
