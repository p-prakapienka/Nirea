// References (T*) are parameter types only (§7.3).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    Engine* other = engine; // error
}
