// A move followed by break leaves the variable moved after the loop (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    int attempts = 0;
    while (attempts < 3) {
        Engine taken = engine;
        break;
    }
    println(engine.power); // error
}
