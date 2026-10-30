// this is a reference and can't be passed by value (§7.3).

struct Engine {
    int power;

    void register() {
        keep(this); // error
    }
}

void keep(Engine engine) {
    println(engine.power);
}

void main() {
    Engine engine;
    engine.register();
}
