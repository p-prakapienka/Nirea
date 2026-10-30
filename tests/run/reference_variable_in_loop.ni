// A reference variable declared in a loop body is new on every iteration, so
// using its owner later in the body doesn't affect the next one (§9.3).

struct Counter {
    int value;
}

void main() {
    Counter[2] counters;
    for (int i = 0; i < 4; i++) {
        Counter* counter = counters[i % 2];
        counter.value = counter.value + i;
        counters[0].value = counters[0].value + 0;
    }
    println(counters[0].value);       // expect: 2
    println(counters[1].value);       // expect: 4
    for (Counter* first = counters[0]; first.value < 5; first.value++) {
        println(first.value);         // expect: 2
    }                                 // expect: 3
                                      // expect: 4
}
