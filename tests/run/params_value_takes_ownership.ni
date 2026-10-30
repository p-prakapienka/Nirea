// A value parameter receives a copy of its argument and can store it; the
// caller's object stays usable (§7.3).

struct Engine {
    int power;

    Engine(int power) {
        this.power = power;
    }
}

struct Vehicle {
    Engine engine;

    void setEngine(Engine engine) {
        this.engine = engine;
    }
}

void main() {
    Engine engine = Engine(150);
    Vehicle vehicle;
    vehicle.setEngine(engine);
    println(vehicle.engine.power);    // expect: 150
    engine.power = 1;
    println(engine.power);            // expect: 1
    println(vehicle.engine.power);    // expect: 150
}
