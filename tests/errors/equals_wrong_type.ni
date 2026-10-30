// equals compares two values of the same type (§6.9).

struct Engine {
    int power;
}

struct Wheel {
    int size;
}

void main() {
    Engine engine;
    Wheel wheel;
    println(engine.equals(wheel)); // error
}
