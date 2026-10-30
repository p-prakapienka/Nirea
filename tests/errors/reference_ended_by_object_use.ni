// Using the referenced object directly, not through the reference, ends the
// reference (§9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle vehicle;
    Engine* engine = vehicle.engine;
    println(vehicle.engine.power);
    engine.power = 1; // error: the reference ended
}
