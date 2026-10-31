// A local variable can be declared without a value and assigned later, on
// every path before it is used, whatever its type (§6.8).

struct Circle {
    float radius;

    Circle(float radius) {
        this.radius = radius;
    }
}

void main() {
    boolean big = true;
    Circle circle;
    if (big) {
        circle = Circle(10.0);
    } else {
        circle = Circle(1.0);
    }
    println(circle.radius);         // expect: 10.0

    int count;
    String label;
    int[] pair;
    count = 3;
    label = "three";
    pair = int[2];
    pair[1] = count;
    println(label);                 // expect: three
    println(pair[1]);               // expect: 3

    Circle unused;
    for (int i = 0; i < 2; i++) {
        Circle inner;
        inner = Circle(2.0);
        println(inner.radius);      // expect: 2.0
    }                               // expect: 2.0
}
