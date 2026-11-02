// Using the owner of the referenced object, such as calling a method on it,
// ends the reference (§9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;

    void service() {
        speed = 0;
    }
}

void main() {
    Vehicle vehicle = Vehicle();
    Engine* engine = vehicle.engine;
    engine.power = 120;
    vehicle.service();
    engine.power = 130; // error: the reference ended
}
