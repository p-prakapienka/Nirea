// A call can't pass the same object to two reference parameters (§9.3).

struct Counter {
    int value;
}

void addFrom(Counter* target, Counter* source) {
    target.value = target.value + source.value;
}

void main() {
    Counter counter = Counter();
    addFrom(counter, counter.copy());
    addFrom(counter, counter); // error: this call already reaches counter through argument 1
}
