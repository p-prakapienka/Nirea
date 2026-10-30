// A switch branch that gives away a referenced local is rejected (§7.3, §10.1).

struct Box {
    int x;
}

void both(Box* first, Box second) {
    println(first.x + second.x);
}

void main() {
    Box a;
    Box b;
    int k = 1;
    both(a, switch (k) { case 1 -> a; default -> b; }); // error
}
