// An array of a struct with a constructor, initialised from constructor
// calls (§5.4, §6.6).

struct Vec2 {
    float x;
    float y;

    Vec2(float x, float y) {
        this.x = x;
        this.y = y;
    }
}

void main() {
    Vec2[2] corners = {Vec2(1.0, 2.5), Vec2(-3.0, 1.0)};
    println(corners[0].y);          // expect: 2.5
    println(corners[1].x);          // expect: -3.0
    println(corners.length);        // expect: 2
}
