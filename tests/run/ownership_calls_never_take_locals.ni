// Calls copy value arguments, so a local stays owned while the same call
// refers to it, and after the call (§7.3).

struct Box {
    int x;

    void show(int u) {
        println(x + u);
    }
}

int consume(Box box) {
    return box.x + 1;
}

boolean big(Box box) {
    return box.x > 0;
}

void look(Box* box, int n) {
    println(box.x + n);
}

void main() {
    Box a = Box();
    a.x = 5;
    look(a, consume(a));                 // expect: 11
    a.show(consume(a));                  // expect: 11
    a.x = consume(a);
    println(a.x);                        // expect: 6
    int k = 0;
    if (k > 0 || big(a)) {
        println("big");                  // expect: big
    }
    println(a.x);                        // expect: 6
}
