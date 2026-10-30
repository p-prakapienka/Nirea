// equals is built in and can't be declared as a method (§7.2).

struct Engine {
    int power;

    boolean equals(Engine* other) { // error
        return power == other.power;
    }
}

void main() {
    Engine engine;
    println(engine.power);
}
