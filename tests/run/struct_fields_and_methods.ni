// Struct fields and methods. Inside a method, fields and other methods can be
// used by name or through this, and this.x and x are the same field (§5.2).

struct Counter {
    int count;
    int step;

    void increment() {
        count = count + step;
    }

    void incrementTwice() {
        increment();
        this.increment();
    }

    void setThroughThis(int value) {
        this.count = value;
    }

    int readByName() {
        return count;
    }
}

void main() {
    Counter counter;
    counter.step = 3;
    counter.increment();
    println(counter.count);         // expect: 3
    counter.incrementTwice();
    println(counter.readByName());  // expect: 9
    counter.setThroughThis(42);
    println(counter.readByName());  // expect: 42
    println(counter.count);         // expect: 42
}
