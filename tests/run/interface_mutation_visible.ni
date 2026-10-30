// An interface-typed parameter is a non-owning reference: changes made through
// it are visible to the caller, whether the struct is a local, a field or an
// array element (§5.3).

interface Counter {
    void increment();
    int value();
}

struct Clicker : Counter {
    int clicks;

    void increment() {
        clicks++;
    }

    int value() {
        return clicks;
    }
}

struct Panel {
    Clicker clicker;
}

void incrementThrice(Counter counter) {
    counter.increment();
    counter.increment();
    counter.increment();
    println(counter.value());
}

void main() {
    Clicker clicker;
    incrementThrice(clicker);       // expect: 3
    println(clicker.clicks);        // expect: 3

    Panel panel;
    incrementThrice(panel.clicker); // expect: 3
    println(panel.clicker.value()); // expect: 3

    Clicker[2] clickers;
    incrementThrice(clickers[1]);   // expect: 3
    incrementThrice(clickers[1]);   // expect: 6
    println(clickers[0].clicks);    // expect: 0
}
