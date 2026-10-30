// An alias is evaluated again at each use, so using it after the variable it
// refers to has given its object away is a use after ownership transfer (§7.1, §9).

struct Engine {
    int power;
}

void main() {
    Engine engine;
    alias power = engine.power;
    Engine other = engine;
    println(power); // error
}
