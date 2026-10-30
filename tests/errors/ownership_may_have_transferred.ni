// Using a variable that may have transferred ownership on some path is rejected (§7.1).

struct Engine {
    int power;
}

void check(boolean ready) {
    Engine first = Engine();
    if (ready) {
        Engine second = first;
        println(second.power);
    }
    println(first.power); // error
}

void main() {
    check(true);
}
