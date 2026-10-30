// Passing a reference, or a call that returns one, to a value parameter
// passes a copy; the referenced object is unchanged (§7.3, §9).

struct Engine {
    int power;
    String name;
}

struct Vehicle {
    Engine engine;

    Engine* getEngine() {
        return engine;
    }
}

int boosted(Engine engine) {
    engine.power = engine.power + 100;
    engine.name.append("+");
    return engine.power;
}

void main() {
    Vehicle vehicle;
    vehicle.engine.power = 1;
    vehicle.engine.name = "v";
    Engine* engine = vehicle.engine;
    println(boosted(engine));                 // expect: 101
    println(boosted(vehicle.getEngine()));    // expect: 101
    println(vehicle.engine.power);            // expect: 1
    println(vehicle.engine.name);             // expect: v
}
