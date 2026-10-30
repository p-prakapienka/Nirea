// A reference parameter can refer to a field or an array element in place;
// the object stays part of its owner and changes are visible there (§7.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
    Engine[2] spares;
}

void install(Engine* engine) {
    engine.power = engine.power + 10;
}

void main() {
    Vehicle vehicle;
    install(vehicle.engine);
    install(vehicle.spares[1]);
    install(vehicle.spares[1]);
    println(vehicle.engine.power);      // expect: 10
    println(vehicle.spares[0].power);   // expect: 0
    println(vehicle.spares[1].power);   // expect: 20
}
