// The object a method is called on and its reference arguments must not
// overlap: an object and a part of it reach the same memory (§9.3).

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
    vehicle.install(engine); // error: this call already reaches vehicle.engine
}
