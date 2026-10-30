// Aliases shorten access chains without creating variables or copies; an
// alias can refer to another alias (§9).

struct Engine {
    private int power;

    int getPower() {
        return power;
    }

    void setPower(int power) {
        this.power = power;
    }
}

struct Vehicle {
    Engine engine;
}

void process(Vehicle vehicle) {
    alias engine = vehicle.engine;
    alias power = engine.getPower();

    println(power);
}

void main() {
    Vehicle vehicle;
    vehicle.engine.setPower(75);
    process(vehicle);                       // expect: 75
    alias engine = vehicle.engine;
    engine.setPower(80);
    println(vehicle.engine.getPower());     // expect: 80
    Engine spare = engine.copy();
    spare.setPower(1);
    println(engine.getPower());             // expect: 80
}
