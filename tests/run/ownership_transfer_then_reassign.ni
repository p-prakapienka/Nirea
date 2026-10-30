// Assignment transfers ownership of a struct. Assigning a new value to the old owner makes
// it usable again (§7.1).

struct Engine {
    int power;
}

Engine createEngine(int power) {
    Engine engine;
    engine.power = power;
    return engine;
}

void main() {
    Engine first = createEngine(10);
    Engine second = first;
    println(second.power);          // expect: 10
    first = createEngine(20);
    println(first.power);           // expect: 20
    second = first;
    println(second.power);          // expect: 20
    first = second.copy();
    first.power = 30;
    println(first.power);           // expect: 30
    println(second.power);          // expect: 20
}
