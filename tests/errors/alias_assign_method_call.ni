// A method call returning a value is not an assignable location, so an alias
// of one can't be assigned to (§9).

struct Engine {
    int power;

    int getPower() {
        return power;
    }
}

void main() {
    Engine engine;
    alias power = engine.getPower();
    power = 5; // error
}
