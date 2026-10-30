// Only a whole local variable can be moved; a field can't change owner (§7.1).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Vehicle vehicle;
    Engine spare = vehicle.engine; // error
}
