// An alias can't be passed by value (§7.3, §9).

struct Engine {
    int power;
}

void install(Engine engine) {
    println(engine.power);
}

void main() {
    Engine engine;
    alias current = engine;
    install(current); // error
}
