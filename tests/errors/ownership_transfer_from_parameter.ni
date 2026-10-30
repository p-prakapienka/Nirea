// A reference parameter refers to the caller's object, which stays with its owner (§7.1, §7.3).

struct Engine {
    int power;
}

void keep(Engine* engine) {
    Engine mine = engine; // error
}

void main() {
    Engine engine;
    keep(engine);
}
