// Assigning a local that is used afterwards copies it: both stay usable, and
// changing one doesn't change the other (§7.1, §7.2).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Engine first = Engine();
    first.power = 1;
    Engine second = first;
    second.power = 2;
    println(first.power);           // expect: 1
    println(second.power);          // expect: 2

    String text = "a";
    String other = text;
    other.append("b");
    println(text);                  // expect: a
    println(other);                 // expect: ab

    int[] values = {1, 2};
    int[] more = values;
    more[0] = 10;
    println(values[0]);             // expect: 1

    Vehicle vehicle = Vehicle();
    vehicle.engine = first;
    vehicle.engine.power = 5;
    println(first.power);           // expect: 1

    Engine[] pair = {first, first};
    pair[0].power = 7;
    println(pair[1].power);         // expect: 1
    println(first.power);           // expect: 1
}
