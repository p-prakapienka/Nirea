// A global object can't be copied or compared (§5.6).

struct Store {
    int count;
}

Store STORE = Store();

void main() {
    Store copy = STORE; // error: can only be used to call its methods
    boolean same = STORE.equals(copy); // error: can't be compared
}
