// Moving a variable declared outside a loop inside the loop body uses it
// after the move on the next iteration (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    for (int i = 0; i < 3; i++) {
        Engine taken = engine; // error
    }
}
