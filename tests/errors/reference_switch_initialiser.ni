// A reference variable names the object it refers to directly; a switch
// expression can't choose it (§9.1).

struct Engine {
    int power;
}

void main() {
    Engine first;
    Engine second;
    int choice = 1;
    Engine* chosen = switch (choice) { // error: not choose one with a switch expression
        case 1 -> first;
        default -> second;
    };
}
