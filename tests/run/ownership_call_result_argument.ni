// A reference parameter doesn't take ownership of its argument, and a call
// result passed as an argument lives until the statement ends (§7.1, §7.3).

struct Engine {
    int power;
}

Engine createEngine(int power) {
    Engine engine = Engine();
    engine.power = power;
    return engine;
}

void inspect(Engine* engine) {
    println(engine.power);
}

void main() {
    inspect(createEngine(42));      // expect: 42
    println(createEngine(7).power); // expect: 7
    Engine engine = createEngine(1);
    inspect(engine);                // expect: 1
    inspect(engine);                // expect: 1
    println(engine.power);          // expect: 1
}
