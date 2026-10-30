// Private methods can be called only inside the methods and constructor of
// their struct (§8).

struct Engine {
    int power;

    private int doubled() {
        return power * 2;
    }
}

void main() {
    Engine engine;
    int value = engine.doubled(); // error
}
