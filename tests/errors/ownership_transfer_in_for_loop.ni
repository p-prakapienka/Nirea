// Giving away the object of a variable declared outside a loop, inside the loop
// body, uses the variable after the transfer on the next iteration (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    for (int i = 0; i < 3; i++) {
        Engine taken = engine; // error
    }
}
