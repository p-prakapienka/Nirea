// A function can't return a reference to a temporary object (§9.2).

struct Engine {
    int power;
}

Engine* make() {
    return Engine(); // error: must refer to an existing object
}

void main() {
    println(1);
}
