// A switch expression whose branches create new objects can be used directly (§10.1).

struct Counter {
    int value;

    Counter(int value) {
        this.value = value;
    }
}

void main() {
    boolean big = true;
    println((switch (big) { case true -> Counter(10); default -> Counter(1); }).value); // expect: 10
    Counter a = Counter(5);
    Counter chosen = switch (big) { case true -> a; default -> Counter(0); };
    println(chosen.value);    // expect: 5
}
