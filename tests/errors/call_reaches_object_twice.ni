// A call can't change an object that it also receives through another
// reference parameter (§9.3).

struct Counter {
    int value;
}

void addFrom(Counter* target, Counter* source) {
    target.value = target.value + source.value;
}

void main() {
    Counter counter = Counter();
    Counter other = counter;
    addFrom(counter, other);
    addFrom(counter, counter); // error: this call changes counter (argument 1)
}
