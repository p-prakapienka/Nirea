// A value owned by an object can't be passed by value; the caller must pass a copy (§7.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void install(Engine engine) {
    println(engine.power);
}

void main() {
    Vehicle vehicle;
    install(vehicle.engine); // error
}
