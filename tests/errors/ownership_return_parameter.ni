// Returning a reference parameter would take the caller's object from its owner; a function must return a copy (§7.3).

struct Engine {
    int power;
}

Engine pass(Engine* engine) {
    return engine; // error
}

void main() {
    Engine engine;
    Engine other = pass(engine);
}
