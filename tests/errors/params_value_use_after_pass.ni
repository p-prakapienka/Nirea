// Passing a local by value moves it; the caller can't use it afterwards (§7.3).

struct Engine {
    int power;
}

void install(Engine engine) {
    println(engine.power);
}

void main() {
    Engine engine;
    install(engine);
    println(engine.power); // error
}
