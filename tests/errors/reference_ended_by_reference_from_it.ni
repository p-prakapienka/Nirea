// Declaring a reference from another reference ends the first one: after
// `Engine* b = a;`, `a` can't be used (§9.3).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    Engine* a = engine;
    Engine* b = a;
    b.power = 1;
    a.power = 2; // error: the reference ended
}
