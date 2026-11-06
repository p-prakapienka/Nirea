// A call that changes the object it is called on can't also receive a part
// of it by reference (§9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;

    void install(Engine* engine) {
        this.engine.power = engine.power;
    }
}

void main() {
    Vehicle vehicle = Vehicle();
    Engine* engine = vehicle.engine;
    vehicle.install(engine); // error: this call changes vehicle (the object it is called on)
}
