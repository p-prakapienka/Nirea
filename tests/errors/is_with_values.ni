// `is` compares references, not values (§6.10).

struct Engine {
    int power;
}

void main() {
    Engine a;
    Engine b;
    println(a is b); // error
}
