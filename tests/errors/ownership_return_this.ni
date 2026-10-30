// Returning this would take the object from its owner through a reference (§7.1).

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
