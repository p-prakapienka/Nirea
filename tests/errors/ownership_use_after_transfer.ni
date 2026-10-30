// Using a variable after it has transferred ownership of its object is rejected (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine first = Engine();
    Engine second = first;
    println(second.power);
    println(first.power); // error
}
