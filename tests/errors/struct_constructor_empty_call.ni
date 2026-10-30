// T() creates a zero value only for a struct that declares no constructor (§5.4).

struct Vec2 {
    float x;
    float y;

    Vec2(float x, float y) {
        this.x = x;
        this.y = y;
    }
}

void main() {
    Vec2 position = Vec2(); // error
}
