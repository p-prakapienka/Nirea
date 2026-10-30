// A reference declared before a loop ends on the first iteration that uses
// its object another way, so the next iteration can't use it (§9.3).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    Engine* reference = engine;
    for (int i = 0; i < 3; i++) {
        reference.power = i; // error: may have ended on some path
        Engine copy = engine.copy();
    }
}
