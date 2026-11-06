// Any number of references can read an object at once, and reading through
// the owner doesn't end them (§9.3).

struct Engine {
    int power;

    int getPower() {
        return power;
    }
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle vehicle = Vehicle();
    vehicle.engine.power = 90;
    Engine* a = vehicle.engine;
    Engine* b = vehicle.engine;
    println(a.power + b.getPower());        // expect: 180
    println(vehicle.engine.getPower());     // expect: 90
    println(a.equals(b));                   // expect: true
    println(a.power);                       // expect: 90
}
