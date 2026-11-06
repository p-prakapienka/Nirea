// A struct type that has a global object can still have ordinary local
// values (§5.6).

struct Tally {
    private int total;

    void add(int amount) {
        total = total + amount;
    }

    int get() {
        return total;
    }
}

Tally TALLY = Tally();

void main() {
    Tally local = Tally();
    local.add(5);
    TALLY.add(2);
    println(local.get());           // expect: 5
    println(TALLY.get());           // expect: 2
}
