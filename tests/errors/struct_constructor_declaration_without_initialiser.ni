// Values of a struct that declares a constructor must come from that
// constructor, so a declaration without an initialiser is rejected (§5.4).

struct Vec2 {
    float x;
    float y;

    Vec2(float x, float y) {
        this.x = x;
        this.y = y;
    }
}

void main() {
    Vec2 position; // error
}
