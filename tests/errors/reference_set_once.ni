// A reference variable is set when it is declared and can't be assigned as a
// whole afterwards (§9.1).

struct Engine {
    int power;
}

void main() {
    Engine first;
    Engine second;
    Engine* engine = first;
    engine.power = 1;
    engine = second; // error: a reference is set when it is declared
}
