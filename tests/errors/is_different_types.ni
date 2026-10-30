// `is` needs two references of the same type (§6.10).

struct Engine {
    int power;
}

struct Wheel {
    int size;
}

boolean same(Engine* engine, Wheel* wheel) {
    return engine is wheel; // error
}

void main() {
    Engine engine;
    Wheel wheel;
    println(same(engine, wheel));
}
