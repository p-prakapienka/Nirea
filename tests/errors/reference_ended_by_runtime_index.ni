// An element whose index is computed at run time may be any element, so using
// it ends references to every element of the array (§9.3).

struct Engine {
    int power;
}

void main() {
    Engine[] engines = Engine[3];
    int i = 2;
    Engine* first = engines[0];
    engines[i].power = 5;
    first.power = 1; // error: the reference ended
}
