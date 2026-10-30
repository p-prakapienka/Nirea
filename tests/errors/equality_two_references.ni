// == only compares values; what two references refer to is compared with equals (§6.9).

struct Engine {
    int power;
}

boolean same(Engine* a, Engine* b) {
    return a == b; // error
}

void main() {
    Engine first;
    Engine second;
    println(same(first, second));
}
