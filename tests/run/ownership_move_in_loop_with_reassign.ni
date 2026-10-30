// A variable moved inside a loop body and reassigned before the next
// iteration stays usable (§7.1).

struct Engine {
    int power;
}

Engine createEngine(int power) {
    Engine engine;
    engine.power = power;
    return engine;
}

void main() {
    Engine engine = createEngine(1);
    int i = 0;
    while (i < 3) {
        Engine current = engine;
        println(current.power);
        engine = createEngine(current.power + 1);
        i++;
    }
    println(engine.power);
}

// expect: 1
// expect: 2
// expect: 3
// expect: 4
