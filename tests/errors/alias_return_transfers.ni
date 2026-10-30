// An alias can't be the source of an ownership transfer, including a
// return value (§9, rule 6).

struct Engine {
    int power;
}

Engine build() {
    Engine engine;
    alias current = engine;
    return current; // error
}

void main() {
    Engine engine = build();
}
