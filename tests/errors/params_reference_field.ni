// A field can't be a reference (§7.3).

struct Engine {
    int power;
}

struct Vehicle {
    Engine* engine; // error
}

void main() {
    println(1);
}
