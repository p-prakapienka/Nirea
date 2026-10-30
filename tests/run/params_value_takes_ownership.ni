// A value parameter takes ownership of its argument and can store it (§7.3).

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
}
