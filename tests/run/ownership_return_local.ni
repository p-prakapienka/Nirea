// Returning a local variable transfers ownership to the caller (§7.1).

struct Engine {
    int power;
}

struct Vehicle {
    String name;
    Engine engine;
}

Engine buildEngine(int power) {
    Engine engine = Engine();
    engine.power = power;
    return engine;
}

Vehicle buildVehicle() {
    Engine engine = buildEngine(150);
    Vehicle vehicle = Vehicle();
    vehicle.engine = engine;
    vehicle.name = "car";
    return vehicle;
}

void main() {
    Vehicle vehicle = buildVehicle();
    println(vehicle.name);                  // expect: car
    println(vehicle.engine.power);          // expect: 150
    println(buildVehicle().engine.power);   // expect: 150
}
