// A global object is a struct value that belongs to the program, used by
// calling its methods; it keeps its state between calls (§5.6).

struct Counter {
    private int[] counts;

    Counter() {
        counts = int[1];
    }

    void add(int key) {
        if (key >= counts.length) {
            int[] bigger = int[key + 1];
            for (int i = 0; i < counts.length; i++) {
                bigger[i] = counts[i];
            }
            counts = bigger;
        }
        counts[key]++;
    }

    int get(int key) {
        return counts[key];
    }
}

Counter COUNTER = Counter();

void record() {
    COUNTER.add(3);
}

void main() {
    record();
    record();
    COUNTER.add(0);
    println(COUNTER.get(3));        // expect: 2
    println(COUNTER.get(0));        // expect: 1
}
