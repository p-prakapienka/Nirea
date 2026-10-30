// == only compares values; two references are compared with `is` (§6.9, §6.10).

struct Engine {
    int power;
}

boolean same(Engine* a, Engine* b) {
    return a == b; // error
}

void main() {
    Engine engine;
    println(same(engine, engine));
}
