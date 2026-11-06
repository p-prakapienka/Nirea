// A call can receive the same object more than once if it only reads it
// (§9.3).

struct Counter {
    int value;

    boolean isAbove(Counter* other) {
        return value > other.value;
    }
}

int total(Counter* first, Counter* second) {
    return first.value + second.value;
}

void main() {
    Counter counter = Counter();
    counter.value = 4;
    println(total(counter, counter));       // expect: 8
    println(counter.isAbove(counter));      // expect: false
}
