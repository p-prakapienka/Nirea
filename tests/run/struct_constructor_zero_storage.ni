// Fields and array elements of a struct type with a constructor start at their
// zero value, including in T[N](), and T() calls a zero-parameter constructor
// (§5.4, §6.6, §6.8).

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
    Vehicle vehicle = Vehicle();
    println(vehicle.engine.power); // expect: 0
    Engine[2] spares = Engine[2]();
    println(spares[1].power);      // expect: 0
    Clock clock = Clock();
    println(clock.ticks);          // expect: 60
}
