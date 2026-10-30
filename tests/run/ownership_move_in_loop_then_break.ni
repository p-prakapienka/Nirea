// A move followed by break never reaches another iteration, so it isn't a
// use after move: moves are tracked through all control flow (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine engine;
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
