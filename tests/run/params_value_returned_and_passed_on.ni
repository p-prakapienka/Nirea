// A value parameter can be modified, returned and passed on by value (§7.3).

struct Engine {
    int power;

    Engine(int power) {
        this.power = power;
    }
}

Engine tune(Engine engine) {
    engine.power = engine.power + 1;
    return engine;
}

Engine tuneTwice(Engine engine) {
    Engine once = tune(engine);
    return tune(once);
}

void main() {
    Engine engine = tuneTwice(Engine(1));
    println(engine.power);            // expect: 3
    Engine other = tune(engine);
    println(other.power);             // expect: 4
}
