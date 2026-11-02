// A global object's initialiser can't reach the object itself, which doesn't
// exist yet (§5.6).

struct Counter {
    int value;

    Counter(int value) {
        this.value = value;
    }

    int get() {
        return value;
    }
}

int start() {
    return SELF.get();
}

Counter SELF = Counter(start()); // error: its initialiser can reach 'SELF' itself

void main() {
    println(SELF.get());
}
