// A constructor can store the copy it receives through a value parameter (§5.4, §7.3).

struct Engine {
    int power;

    Engine(int power) {
        this.power = power;
    }
}

struct Vehicle {
    Engine engine;
    int[] wheels;

    Vehicle(Engine engine, int[] wheels) {
        this.engine = engine;
        this.wheels = wheels;
    }
}

void main() {
    int[] wheels = {17, 18};
    Vehicle vehicle = Vehicle(Engine(90), wheels);
    println(vehicle.engine.power);    // expect: 90
    println(vehicle.wheels[1]);       // expect: 18
}
