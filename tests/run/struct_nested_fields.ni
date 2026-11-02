// A struct owns nested structs stored inline; nested fields and methods are
// reached through access chains (§5.2, §7.4).

struct Engine {
    int power;

    void boost(int amount) {
        power += amount;
    }
}

struct Vehicle {
    String name;
    Engine engine;
}

void main() {
    Vehicle car = Vehicle();
    car.name = "car";
    car.engine.power = 100;
    car.engine.boost(20);
    println(car.engine.power);      // expect: 120

    Engine replacement = Engine();
    replacement.power = 7;
    car.engine = replacement;
    println(car.engine.power);      // expect: 7
    println(car.name);              // expect: car
}
