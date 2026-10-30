// A move on a path that returns doesn't affect the code after it (§7.1).

struct Engine {
    int power;
}

Engine createEngine(int power) {
    Engine engine;
    engine.power = power;
    return engine;
}

int consume(boolean early) {
    Engine engine = createEngine(5);
    if (early) {
        Engine moved = engine;
        return moved.power;
    }
    return engine.power + 1;
}

void main() {
    println(consume(true));         // expect: 5
    println(consume(false));        // expect: 6
}
