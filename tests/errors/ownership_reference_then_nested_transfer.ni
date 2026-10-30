// A call can't give away a local inside a nested call while it refers to that local (§7.3).

struct Box {
    int x;
}

int consume(Box box) {
    return box.x;
}

void look(Box* box, int n) {
    println(box.x + n);
}

void main() {
    Box a;
    look(a, consume(a)); // error
}
