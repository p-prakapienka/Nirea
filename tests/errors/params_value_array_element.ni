// An array element belongs to its array and can't be passed by value (§7.3).

struct Engine {
    int power;
}

void install(Engine engine) {
    println(engine.power);
}

void main() {
    Engine[2] engines;
    install(engines[0]); // error
}
