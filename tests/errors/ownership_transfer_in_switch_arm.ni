// An ownership transfer inside one switch arm means the variable may no longer own its object after
// the switch (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine engine = Engine();
    int mode = 1;
    switch (mode) {
        case 1 -> {
            Engine taken = engine;
        }
        default -> println("none");
    }
    println(engine.power); // error
}
