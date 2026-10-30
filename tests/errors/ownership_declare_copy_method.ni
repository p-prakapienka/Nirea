// copy() is built in; a struct can't declare its own method named copy (§7.2).

struct Engine {
    int power;

    Engine copy() { // error
        Engine result;
        result.power = power;
        return result;
    }
}

void main() {
    Engine engine;
}
