// copy() makes a deep, independent copy: owned fields and elements are
// copied too (§7.2).

struct Engine {
    int power;
}

struct Vehicle {
    String name;
    Engine engine;
    int[2] wheels;
    Engine[2] spares;
}

void main() {
    Vehicle original;
    original.name = "first";
    original.engine.power = 100;
    original.wheels[0] = 1;
    original.spares[1].power = 7;

    Vehicle duplicate = original.copy();
    duplicate.name = "second";
    duplicate.engine.power = 200;
    duplicate.wheels[0] = 2;
    duplicate.spares[1].power = 8;

    println(original.name);                 // expect: first
    println(original.engine.power);         // expect: 100
    println(original.wheels[0]);            // expect: 1
    println(original.spares[1].power);      // expect: 7
    println(duplicate.name);                // expect: second
    println(duplicate.engine.power);        // expect: 200
    println(duplicate.wheels[0]);           // expect: 2
    println(duplicate.spares[1].power);     // expect: 8
}
