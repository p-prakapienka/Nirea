// A stored reference returned by a call reaches the object the call was made
// on as a whole, so using any part of it ends the reference (§9.2, §9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;

    Engine* getEngine() {
        return engine;
    }
}

void main() {
    Vehicle vehicle;
    Engine* engine = vehicle.getEngine();
    vehicle.speed = 1;
    engine.power = 1; // error: the reference ended
}
