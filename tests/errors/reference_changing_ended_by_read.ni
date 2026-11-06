// A reference used to change its object must be the only way to it while it
// is in use, so even reading the object another way ends it (§9.3).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    Engine* writer = engine;
    writer.power = 1;
    println(engine.power);
    writer.power = 2; // error: the reference ended
}
