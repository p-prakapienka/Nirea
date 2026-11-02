// Using a part of the owner that doesn't overlap the referenced object keeps
// the reference usable; so do array elements with different constant
// indexes (§9.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    int speed;
    Engine[] spares;

    Vehicle() {
        spares = Engine[2];
    }
}

void main() {
    Vehicle vehicle = Vehicle();
    Engine* engine = vehicle.engine;
    vehicle.speed = 50;
    engine.power = 120;
    println(vehicle.speed);           // expect: 50

    Engine* first = vehicle.spares[0];
    Engine* second = vehicle.spares[1];
    first.power = 1;
    second.power = 2;
    println(vehicle.spares[0].power); // expect: 1
    println(vehicle.spares[1].power); // expect: 2
    println(vehicle.engine.power);    // expect: 120
}
