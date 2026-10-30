// The same value can be passed to several reference parameters of one call (§7.3).

struct Counter {
    int value;
}

void addFrom(Counter* target, Counter* source) {
    target.value = target.value + source.value;
}

void main() {
    Counter counter;
    counter.value = 5;
    addFrom(counter, counter);
    println(counter.value);   // expect: 10
}
