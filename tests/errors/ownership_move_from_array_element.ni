// An array element can't be moved out of its array (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine[2] engines;
    Engine first = engines[0]; // error
}
