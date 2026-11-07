// A local that is used afterwards on any path is copied: after a branch, a
// switch arm, a switch expression or a loop that breaks (§7.2).

struct Engine {
    int power;
}

int pick(int choice) {
    Engine small = Engine();
    small.power = 1;
    Engine large = Engine();
    large.power = 9;
    Engine chosen = switch (choice) {
        case 1 -> small;
        default -> large;
    };
    chosen.power = chosen.power + 100;
    return small.power + large.power;
}

void main() {
    Engine engine = Engine();
    engine.power = 3;
    boolean ready = true;
    if (ready) {
        Engine taken = engine;
        taken.power = 4;
    }
    println(engine.power);          // expect: 3

    int mode = 1;
    switch (mode) {
        case 1 -> {
            Engine taken = engine;
            taken.power = 5;
        }
        default -> println("none");
    }
    println(engine.power);          // expect: 3

    println(pick(1));               // expect: 10

    while (true) {
        Engine taken = engine;
        taken.power = 6;
        break;
    }
    println(engine.power);          // expect: 3
}
