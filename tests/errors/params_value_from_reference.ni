// A reference parameter can't be passed on by value (§7.3).

struct Engine {
    int power;
}

void install(Engine engine) {
    println(engine.power);
}

void forward(Engine* engine) {
    install(engine); // error
}

void main() {
    Engine engine;
    forward(engine);
}
