// A value parameter is an owned local: it can be assigned as a whole (§7.3).

struct Engine {
    int power;
}

int replaced(Engine engine) {
    Engine fresh;
    fresh.power = 9;
    engine = fresh;
    return engine.power;
}

void main() {
    Engine engine;
    engine.power = 1;
    println(replaced(engine));        // expect: 9
}
