// A method's receiver is borrowed, so it can't also be passed by value to the same call (§7.3).

struct Engine {
    int power;

    void merge(Engine other) {
        power = power + other.power;
    }
}

void main() {
    Engine engine;
    engine.merge(engine); // error
}
