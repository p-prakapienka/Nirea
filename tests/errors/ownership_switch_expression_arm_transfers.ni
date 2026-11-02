// The value of a switch-expression arm used as an initialiser changes owner, so
// each arm's variable may no longer own its object afterwards (§7.1).

struct Engine {
    int power;
}

void choose(int choice) {
    Engine small = Engine();
    Engine large = Engine();
    Engine chosen = switch (choice) {
        case 1 -> small;
        default -> large;
    };
    println(small.power); // error
}

void main() {
    choose(1);
}
