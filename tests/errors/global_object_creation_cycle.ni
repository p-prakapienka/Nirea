// Global objects whose initialisers need each other can't be created in any
// order (§5.6).

struct Counter {
    int value;

    Counter(int value) {
        this.value = value;
    }

    int get() {
        return value;
    }
}

Counter FIRST = Counter(SECOND.get()); // error: can't be created
Counter SECOND = Counter(FIRST.get());

void main() {
    println(FIRST.get());
}
