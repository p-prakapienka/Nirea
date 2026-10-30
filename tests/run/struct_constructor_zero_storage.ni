// Fields and array elements of a struct type with a constructor start at their zero value,
// and T() calls a zero-parameter constructor (§5.4, §6.8).

struct Engine {
    int power;

    Engine(int power) {
        this.power = power;
    }
}

struct Vehicle {
    Engine engine;
}

struct Clock {
    int ticks;

    Clock() {
        ticks = 60;
    }
}

void main() {
    Vehicle vehicle;
    println(vehicle.engine.power); // expect: 0
    Engine[2] spares;
    println(spares[1].power);      // expect: 0
    Clock clock = Clock();
    println(clock.ticks);          // expect: 60
}
