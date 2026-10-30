// An alias can't be the source of an ownership transfer (§7.1, §9).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    alias current = engine;
    Engine other = current; // error
}
