// The compiler works out which methods change `this`: calling one ends the
// references that read the object, calling one that only reads doesn't (§9.3).

struct Engine {
    int power;

    int getPower() {
        return power;
    }

    void boost() {
        power = power + 10;
    }
}

void main() {
    Engine engine = Engine();
    Engine* reader = engine;
    println(engine.getPower());
    println(reader.power);
    engine.boost();
    println(reader.power); // error: the reference ended
}
