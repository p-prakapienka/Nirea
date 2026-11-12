// Assignment copies a struct, so both variables stay usable; assigning again
// replaces the old object (§7.1, §7.5).

struct Engine {
    int power;
}

Engine createEngine(int power) {
    Engine engine = Engine();
    engine.power = power;
    return engine;
}

void main() {
    Engine first = createEngine(10);
    Engine second = first;
    println(second.power);          // expect: 10
    println(first.power);           // expect: 10
    first = createEngine(20);
    println(first.power);           // expect: 20
    second = first;
    println(second.power);          // expect: 20
    first = second;
    first.power = 30;
    println(first.power);           // expect: 30
    println(second.power);          // expect: 20
}
