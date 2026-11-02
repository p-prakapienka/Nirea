// Giving the owner's object to a new owner ends references to it (§7.1, §9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle vehicle = Vehicle();
    Engine* engine = vehicle.engine;
    Vehicle moved = vehicle;
    engine.power = 1; // error: the reference ended
}
