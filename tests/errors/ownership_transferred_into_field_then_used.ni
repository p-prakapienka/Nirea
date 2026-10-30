// Assigning a local to a field transfers ownership; the local can't be used afterwards (§7.1).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Engine engine;
    Vehicle vehicle;
    vehicle.engine = engine;
    println(engine.power); // error
}
