// this is a reference and can't be compared with == (§6.9).

struct Engine {
    int power;

    boolean matches(Engine other) {
        return this == other; // error
    }
}

void main() {
    Engine engine;
    println(engine.matches(Engine()));
}
