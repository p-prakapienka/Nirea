// A move followed by break never reaches another iteration, so the variable
// isn't used afterwards and its object moves (§7.2).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    engine.power = 3;
    for (int i = 0; i < 5; i++) {
        if (i == 2) {
            Engine last = engine;
            println(last.power);
            break;
        }
        println(i);
    }
}

// expect: 0
// expect: 1
// expect: 3
