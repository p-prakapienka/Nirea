// Struct parameters and this are non-owning references: the function can
// write fields and call methods, and the caller sees the changes (§7.3).

struct Engine {
    int power;

    void boost(int amount) {
        power += amount;
    }
}

struct Vehicle {
    Engine engine;
    int wheels;
}

void upgrade(Vehicle vehicle) {
    vehicle.wheels = 4;
    vehicle.engine.power += 50;
    vehicle.engine.boost(1);
}

void tune(Engine engine) {
    engine.power *= 2;
}

void main() {
    Vehicle car;
    car.engine.power = 100;
    upgrade(car);
    println(car.wheels);            // expect: 4
    println(car.engine.power);      // expect: 151
    tune(car.engine);
    println(car.engine.power);      // expect: 302
    car.engine.boost(8);
    println(car.engine.power);      // expect: 310
}
