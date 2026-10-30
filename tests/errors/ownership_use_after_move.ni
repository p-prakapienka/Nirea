// Using a variable after its value has been moved is rejected (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine first;
    Engine second = first;
    println(second.power);
    println(first.power); // error
}
