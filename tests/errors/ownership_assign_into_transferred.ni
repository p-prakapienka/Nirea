// An assignment can't write into a local that its right-hand side gives away (§7.1).

struct Box {
    int x;
}

int consume(Box box) {
    return box.x;
}

void main() {
    Box a;
    a.x = consume(a); // error
}
