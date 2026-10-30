// A reference variable can refer to a reference parameter, `this`, or a part
// of them; using the parameter ends the variable, not the other way round
// (§9.1, §9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;

    void tune() {
        Engine* engine = this.engine;
        engine.power = engine.power + 1;
        speed = speed + 1;
        engine.power = engine.power + 1;
    }
}

void service(Vehicle* vehicle) {
    Engine* engine = vehicle.engine;
    engine.power = engine.power * 10;
    vehicle.tune();
}

void main() {
    Vehicle vehicle;
    vehicle.engine.power = 1;
    service(vehicle);
    println(vehicle.engine.power);    // expect: 12
    println(vehicle.speed);           // expect: 1
}
