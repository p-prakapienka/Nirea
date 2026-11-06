// Changing an object ends the references that read it (§9.3).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    Engine* reader = engine;
    println(reader.power);
    engine.power = 100;
    println(reader.power); // error: the reference ended
}
