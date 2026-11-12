// A local declared outside a loop and assigned inside it is used again on the
// next iteration, so it is copied every time (§7.2).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Engine engine = Engine();
    engine.power = 1;
    Vehicle vehicle = Vehicle();
    for (int i = 0; i < 3; i++) {
        vehicle.engine = engine;
        vehicle.engine.power = vehicle.engine.power + i;
    }
    println(vehicle.engine.power);  // expect: 3
    println(engine.power);          // expect: 1

    String line = "-";
    String[] lines = String[3];
    int n = 0;
    while (n < 3) {
        lines[n] = line;
        lines[n].appendChar('>');
        n++;
    }
    println(lines[2]);              // expect: ->
    println(line);                  // expect: -
}
