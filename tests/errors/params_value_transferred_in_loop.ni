// Passing a local by value inside a loop would transfer it again on every iteration (§7.1, §7.3).

struct Engine {
    int power;
}

void install(Engine engine) {
    println(engine.power);
}

void main() {
    Engine engine;
    for (int i = 0; i < 2; i++) {
        install(engine); // error
    }
}
