// A global object's fields can't be used through its name, even when they
// are public; it is used by calling its methods (§5.6).

struct Store {
    int count;
}

Store STORE = Store();

void main() {
    STORE.count = 1; // error: can't be used through its name
}
