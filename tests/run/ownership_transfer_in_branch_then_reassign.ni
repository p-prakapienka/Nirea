// A variable that gave its object away on some path is usable again once every such path
// has assigned it a new value (§7.1).

struct Engine {
    int power;
}

Engine createEngine(int power) {
    Engine engine;
    engine.power = power;
    return engine;
}

void run(boolean ready) {
    Engine engine = createEngine(1);
    if (ready) {
        Engine taken = engine;
        println(taken.power);
        engine = createEngine(2);
    }
    println(engine.power);

    if (ready) {
        Engine left = engine;
        println(left.power);
    } else {
        Engine right = engine;
        println(right.power);
    }
    engine = createEngine(3);
    println(engine.power);
}

void main() {
    run(true);
    run(false);
}

// expect: 1
// expect: 2
// expect: 2
// expect: 3
// expect: 1
// expect: 1
// expect: 3
