// The result of a call that returns a reference can't be assigned as a whole;
// its fields can (§9.2).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;

    Engine* getEngine() {
        return engine;
    }
}

void main() {
    Vehicle vehicle;
    vehicle.getEngine().power = 1;
    vehicle.getEngine() = Engine(); // error: can't assign to the result of 'getEngine' as a whole
}
