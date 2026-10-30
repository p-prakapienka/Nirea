// A reference variable must be initialised with the object it refers to (§9.1).

struct Engine {
    int power;
}

void main() {
    Engine* engine; // error: must be initialised
}
