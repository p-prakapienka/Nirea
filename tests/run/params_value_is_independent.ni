// Changes to a value parameter aren't visible to the caller; a reference parameter's are (§7.3).

struct Counter {
    int value;
}

void bumpValue(Counter counter) {
    counter.value = counter.value + 100;
    println(counter.value);
}

void bumpReference(Counter* counter) {
    counter.value = counter.value + 1;
}

void main() {
    Counter counter = Counter();
    counter.value = 1;
    bumpValue(counter.copy());        // expect: 101
    println(counter.value);           // expect: 1
    bumpReference(counter);
    println(counter.value);           // expect: 2
}
