// A function can't return a reference (§7.3).

struct Engine {
    int power;
}

Engine* make() { // error
    Engine engine;
    return engine;
}

void main() {
    println(1);
}
