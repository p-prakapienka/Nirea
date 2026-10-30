// A call can't refer to a local and take it by value at the same time (§7.3).

struct Engine {
    int power;
}

void combine(Engine* first, Engine second) {
    println(first.power + second.power);
}

void main() {
    Engine engine;
    combine(engine, engine); // error
}
