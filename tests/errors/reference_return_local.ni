// A function can't return a reference to a local variable, a value parameter
// or a reference to one of them: they are destroyed when it returns (§9.2).

struct Engine {
    int power;
}

Engine* fromLocal() {
    Engine engine;
    return engine; // error: can't return a reference to local variable 'engine'
}

Engine* fromValue(Engine engine) {
    return engine; // error: can't return a reference to value parameter 'engine'
}

Engine* throughReference() {
    Engine engine;
    Engine* reference = engine;
    return reference; // error: can't return a reference to 'reference'
}

void main() {
    println(1);
}
