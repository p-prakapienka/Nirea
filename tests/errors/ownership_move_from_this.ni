// this is a non-owning reference and can't be moved out of (§7.1).

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
