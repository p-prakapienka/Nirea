// Aliases obey normal visibility rules; the expression is resolved where the
// alias is declared (§9, rule 8).

struct Engine {
    private int power;
}

void main() {
    Engine engine;
    alias power = engine.power; // error
}
