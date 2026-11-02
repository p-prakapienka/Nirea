// The Vec2 example from §5.2: float fields and methods that read and write them.

struct Vec2 {
    float x;
    float y;

    float lengthSquared() {
        return x * x + y * y;
    }

    void scale(float factor) {
        x = x * factor;
        y = y * factor;
    }
}

void main() {
    Vec2 v = Vec2();
    v.x = 1.5;
    v.y = 0.5;
    println(v.lengthSquared());     // expect: 2.5
    v.scale(-2.0);
    println(v.x);                   // expect: -3.0
    println(v.y);                   // expect: -1.0
}
