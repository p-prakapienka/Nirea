// A move inside one switch arm means the variable may have been moved after
// the switch (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    int mode = 1;
    switch (mode) {
        case 1 -> {
            Engine taken = engine;
        }
        default -> println("none");
    }
    println(engine.power); // error
}
