// A global object can't be copied or compared (§5.6).

struct Store {
    int count;
}

Store STORE = Store();

void main() {
    Store copy = STORE.copy(); // error: can't be copied or compared
}
