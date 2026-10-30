// A reference variable can't refer to a temporary object, including through a
// call that returns a reference to one (§9.1, §9.2).

struct Engine {
    int power;

    Engine* getSelf() {
        return this;
    }
}

void main() {
    Engine* made = Engine(); // error: must refer to an existing object
    Engine* chained = Engine().getSelf(); // error: must refer to an existing object
    String* text = "abc"; // error: must refer to an existing object
}
