// A reference variable can't give its object to a new owner; copy() gives an
// independent object (§7.1, §9).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    Engine* reference = engine;
    Engine copy = reference.copy();
    Engine other = reference; // error: its object stays with its owner
}
