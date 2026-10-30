// The value of a switch-expression arm used as an initialiser is moved, so
// each arm's variable may have been moved afterwards (§7.1).

struct Engine {
    int power;
}

void choose(int choice) {
    Engine small;
    Engine large;
    Engine chosen = switch (choice) {
        case 1 -> small;
        default -> large;
    };
    println(small.power); // error
}

void main() {
    choose(1);
}
