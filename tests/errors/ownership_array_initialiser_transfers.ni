// An element of an array initialiser gives its object to the array (§7.1).

struct Engine {
    int power;
}

void main() {
    Engine left;
    Engine right;
    Engine[2] pair = {left, right};
    println(left.power); // error
}
