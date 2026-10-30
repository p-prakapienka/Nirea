// A private member can be used only inside the methods and constructor of its
// struct (§8).

struct Engine {
    private int power;

    int getPower() {
        return power;
    }
}

void main() {
    Engine engine;
    println(engine.power); // error
}
