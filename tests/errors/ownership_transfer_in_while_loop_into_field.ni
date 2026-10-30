// Giving an outer variable's object to a field inside a loop transfers it again on the
// next iteration (§7.1).

struct Engine {
    int power;
}

struct Vehicle {
    Engine engine;
}

void main() {
    Engine engine;
    Vehicle vehicle;
    int laps = 0;
    while (laps < 3) {
        vehicle.engine = engine; // error
        laps++;
    }
}
