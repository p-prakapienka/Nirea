// A value parameter receives a copy of any object: a local, a field, an array
// element, a reference parameter or variable, or this; the original is
// unchanged (§7.3).

struct Engine {
    int power;
    String name;

    Engine(int power, String name) {
        this.power = power;
        this.name = name;
    }

    void merge(Engine other) {
        power = power + other.power;
        other.power = 0;
    }

    int boostCopy() {
        return boosted(this);
    }
}

struct Vehicle {
    Engine engine;
    Engine[2] spares;
}

int boosted(Engine engine) {
    engine.power = engine.power + 100;
    engine.name.append("+");
    return engine.power;
}

int fromReference(Engine* engine) {
    return boosted(engine);
}

int combine(Engine* first, Engine second) {
    second.power = second.power * 2;
    return first.power + second.power;
}

void main() {
    Engine engine = Engine(1, "e");
    println(boosted(engine));            // expect: 101
    println(engine.power);               // expect: 1
    println(engine.name);                // expect: e

    Vehicle vehicle;
    vehicle.engine = Engine(2, "v");
    vehicle.spares[1] = Engine(3, "s");
    println(boosted(vehicle.engine));    // expect: 102
    println(boosted(vehicle.spares[1])); // expect: 103
    println(vehicle.engine.power);       // expect: 2
    println(vehicle.spares[1].power);    // expect: 3
    println(fromReference(engine));      // expect: 101
    println(engine.boostCopy());         // expect: 101
    Engine* spare = vehicle.spares[1];
    println(boosted(spare));             // expect: 103

    println(combine(engine, engine));    // expect: 3
    engine.merge(engine);
    println(engine.power);               // expect: 2

    for (int i = 0; i < 2; i++) {
        println(boosted(engine));        // expect: 102
    }                                    // expect: 102
    println(engine.power);               // expect: 2
}
