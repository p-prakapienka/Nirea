// A reference that may have ended on some path can't be used (§9.3).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    boolean ready = true;
    Engine* reference = engine;
    if (ready) {
        engine.power = 1;
    }
    reference.power = 2; // error: may have ended on some path
}
