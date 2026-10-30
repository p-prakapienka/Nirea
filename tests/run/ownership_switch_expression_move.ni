// The value of a switch-expression arm moves when the switch is used as an
// initialiser or a return value (§7.1).

struct Engine {
    int power;
}

Engine createEngine(int power) {
    Engine engine;
    engine.power = power;
    return engine;
}

Engine pick(int choice) {
    Engine small = createEngine(1);
    Engine large = createEngine(9);
    return switch (choice) {
        case 1 -> small;
        default -> large;
    };
}

void main() {
    Engine small = createEngine(2);
    Engine large = createEngine(8);
    int choice = 2;
    Engine chosen = switch (choice) {
        case 1 -> small;
        default -> large;
    };
    println(chosen.power);          // expect: 8
    println(pick(1).power);         // expect: 1
}
