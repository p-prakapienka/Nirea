// A global object can't be used as a value: not assigned, given to a new
// owner or passed as an argument (§5.6).

struct Store {
    int count;
}

void inspect(Store* store) {
    println(store.count);
}

Store STORE = Store();

void main() {
    Store taken = STORE; // error: can only be used to call its methods
    inspect(STORE); // error: can only be used to call its methods
    STORE = Store(); // error: can only be used to call its methods
}
