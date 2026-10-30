// Returning a parameter would move it; a function must return a copy (§7.3).

struct Engine {
    int power;
}

Engine pass(Engine engine) {
    return engine; // error
}

void main() {
    Engine engine;
    Engine other = pass(engine);
}
