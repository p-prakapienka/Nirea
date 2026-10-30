// Returning a field without copy() would take it from its owner (§7.1).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;

    Engine getEngine() {
        return engine; // error
    }
}

void main() {
    Vehicle vehicle;
    Engine engine = vehicle.getEngine();
}
