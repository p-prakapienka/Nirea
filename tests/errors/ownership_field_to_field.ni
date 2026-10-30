// Assigning one struct's field to another's would take a field from its owner (§7.1).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle first;
    Vehicle second;
    second.engine = first.engine; // error
}
