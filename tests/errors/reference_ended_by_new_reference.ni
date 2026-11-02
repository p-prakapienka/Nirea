// Declaring a new reference to an object ends the earlier reference to it (§9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle vehicle = Vehicle();
    Engine* a = vehicle.engine;
    Engine* b = vehicle.engine;
    b.power = 2;
    a.power = 1; // error: the reference ended
}
