// Assigning a new object to the owner ends references to the old one (§9.3).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    Engine* reference = engine;
    engine = Engine();
    reference.power = 1; // error: the reference ended
}
