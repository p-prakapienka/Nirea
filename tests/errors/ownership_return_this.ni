// Returning this would move out of a non-owning reference (§7.1).

struct Engine {
    int power;

    Engine itself() {
        return this; // error
    }
}

void main() {
    Engine engine;
    Engine other = engine.itself();
}
