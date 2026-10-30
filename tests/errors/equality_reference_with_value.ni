// A reference can't be compared with a value (§6.9).

struct Engine {
    int power;
}

boolean matches(Engine* engine) {
    Engine other;
    return engine == other; // error
}

void main() {
    Engine engine;
    println(matches(engine));
}
