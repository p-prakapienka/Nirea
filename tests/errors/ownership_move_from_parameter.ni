// A reference parameter borrows its value, so it can't be moved (§7.1, §7.3).

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
