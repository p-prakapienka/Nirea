// A reference variable refers to an existing object where it is in memory;
// changes through it change the owner's object, and nothing is copied (§9.1).

struct Engine {
    int power;
    String name;

    void rename(String name) {
        this.name = name;
    }
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle vehicle = Vehicle();
    vehicle.engine.power = 100;
    Engine* engine = vehicle.engine;
    engine.power = engine.power + 50;
    engine.rename("turbo");
    engine.name.append("!");
    println(vehicle.engine.power);    // expect: 150
    println(vehicle.engine.name);     // expect: turbo!

    String text = "a";
    String* same = text;
    same.append("b");
    println(text);                    // expect: ab
}
