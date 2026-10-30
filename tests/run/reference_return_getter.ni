// A function can return a reference to a part of `this` or of a reference
// parameter, including through a reference variable (§9.2).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    Engine[3] spares;
    String name;

    Engine* getEngine() {
        return engine;
    }

    Engine* spare(int index) {
        Engine* chosen = spares[index];
        return chosen;
    }

    String* getName() {
        return name;
    }
}

Engine* stronger(Engine* a, Engine* b) {
    if (a.power >= b.power) {
        return a;
    }
    return b;
}

void main() {
    Vehicle vehicle;
    vehicle.getEngine().power = 90;
    vehicle.spare(2).power = 30;
    vehicle.getName().append("car");
    println(vehicle.engine.power);            // expect: 90
    println(vehicle.spares[2].power);         // expect: 30
    println(vehicle.name);                    // expect: car

    Engine first;
    first.power = 5;
    Engine second;
    second.power = 8;
    Engine* winner = stronger(first, second);
    winner.power = winner.power + 1;
    println(second.power);                    // expect: 9
    println(stronger(first, second).power);   // expect: 9
}
