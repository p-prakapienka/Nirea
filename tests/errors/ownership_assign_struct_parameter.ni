// A function can't assign to a struct parameter as a whole (§7.3).

struct Engine {
    int power;
}

void reset(Engine engine) {
    engine = Engine(); // error
}

void main() {
    Engine engine;
    reset(engine);
}
