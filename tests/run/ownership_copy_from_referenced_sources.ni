// Fields, reference parameters, array elements, this and reference variables can't give their objects to a new owner,
// but copy() gives an independent value from each of them (§7.1, §7.2).

struct Engine {
    int power;

    Engine duplicate() {
        return this.copy();
    }
}

struct Vehicle {
    Engine engine;
}

Engine copyParameter(Engine* engine) {
    return engine.copy();
}

void main() {
    Vehicle vehicle;
    vehicle.engine.power = 10;

    Engine fromField = vehicle.engine.copy();
    fromField.power = 11;
    println(vehicle.engine.power);  // expect: 10
    println(fromField.power);       // expect: 11

    Engine fromParameter = copyParameter(vehicle.engine);
    println(fromParameter.power);   // expect: 10

    Engine[2] engines;
    engines[0].power = 20;
    Engine fromElement = engines[0].copy();
    fromElement.power = 21;
    println(engines[0].power);      // expect: 20

    Engine fromThis = vehicle.engine.duplicate();
    println(fromThis.power);        // expect: 10

    Engine* referenced = vehicle.engine;
    Engine fromReference = referenced.copy();
    fromReference.power = 12;
    println(vehicle.engine.power);  // expect: 10

    Vehicle other;
    other.engine = vehicle.engine.copy();
    println(other.engine.power);    // expect: 10
}
