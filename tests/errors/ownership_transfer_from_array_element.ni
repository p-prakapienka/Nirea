// An array element is part of its array and can't be given to a new owner (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine[2] engines;
    Engine first = engines[0]; // error
}
