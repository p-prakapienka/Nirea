// A method's receiver stays referenced while its arguments are evaluated (§7.3).

struct Box {
    int x;

    void show(int u) {
        println(x + u);
    }
}

int consume(Box box) {
    return box.x;
}

void main() {
    Box a;
    a.show(consume(a)); // error
}
