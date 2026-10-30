// this is a non-owning reference; its object can't be given to a new owner (§7.1).

struct Engine {
    int power;

    void keepSelf() {
        Engine kept = this; // error
    }
}

void main() {
    Engine engine;
    engine.keepSelf();
}
