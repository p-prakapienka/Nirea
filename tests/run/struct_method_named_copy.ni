// There is no built-in copy(), so a struct can declare its own method named
// copy (§7.2).

struct Engine {
    int power;

    Engine copy() {
        Engine result = Engine();
        result.power = power + 1;
        return result;
    }
}

void main() {
    Engine engine = Engine();
    engine.power = 1;
    Engine other = engine.copy();
    println(other.power);           // expect: 2
}
