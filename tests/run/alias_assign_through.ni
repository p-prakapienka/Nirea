// An alias of an assignable location can be assigned through (§9).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle vehicle;
    alias power = vehicle.engine.power;
    power = 50;
    println(vehicle.engine.power);  // expect: 50
    power += 5;
    println(vehicle.engine.power);  // expect: 55
    power++;
    println(power);                 // expect: 56

    int[3] values;
    int position = 1;
    alias slot = values[position];
    slot = 7;
    println(values[1]);             // expect: 7

    int total = 0;
    alias sum = total;
    sum = 3;
    println(total);                 // expect: 3

    alias engine = vehicle.engine;
    engine.power = 9;
    println(vehicle.engine.power);  // expect: 9
    engine = Engine();
    println(vehicle.engine.power);  // expect: 0
}
