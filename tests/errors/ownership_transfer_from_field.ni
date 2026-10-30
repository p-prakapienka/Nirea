// Only a whole local variable can give its object to a new owner; a field stays part of its owner (§7.1).

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
