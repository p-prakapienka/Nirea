// Ownership moves into fields, array elements and array initialisers, and a
// call result can be used in any of these positions (§7.1).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

Engine createEngine(int power) {
    Engine engine;
    engine.power = power;
    return engine;
}

void main() {
    Engine first = createEngine(1);
    Vehicle vehicle;
    vehicle.engine = first;
    println(vehicle.engine.power);          // expect: 1

    Engine second = createEngine(2);
    Engine[2] engines;
    engines[1] = second;
    println(engines[1].power);              // expect: 2

    Engine third = createEngine(3);
    Engine fourth = createEngine(4);
    Engine[2] pair = {third, fourth};
    println(pair[0].power + pair[1].power); // expect: 7

    vehicle.engine = createEngine(5);
    println(vehicle.engine.power);          // expect: 5
}
