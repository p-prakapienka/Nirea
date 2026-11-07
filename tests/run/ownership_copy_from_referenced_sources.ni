// Fields, reference parameters, array elements, this and reference variables
// stay with their owners; assigning them gives a copy (§7.1).

struct Engine {
    int power;

    Engine duplicate() {
        return this;
    }
}

struct Vehicle {
    Engine engine;
}

Engine copyParameter(Engine* engine) {
    return engine;
}

void main() {
    Vehicle vehicle = Vehicle();
    vehicle.engine.power = 10;

    Engine fromField = vehicle.engine;
    fromField.power = 11;
    println(vehicle.engine.power);  // expect: 10
    println(fromField.power);       // expect: 11

    Engine fromParameter = copyParameter(vehicle.engine);
    println(fromParameter.power);   // expect: 10

    Engine[] engines = Engine[2];
    engines[0].power = 20;
    Engine fromElement = engines[0];
    fromElement.power = 21;
    println(engines[0].power);      // expect: 20

    Engine fromThis = vehicle.engine.duplicate();
    println(fromThis.power);        // expect: 10

    Engine* referenced = vehicle.engine;
    Engine fromReference = referenced;
    fromReference.power = 12;
    println(vehicle.engine.power);  // expect: 10

    Vehicle other = Vehicle();
    other.engine = vehicle.engine;
    println(other.engine.power);    // expect: 10
}
