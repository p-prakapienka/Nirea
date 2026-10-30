// Methods can't be overloaded: each name is declared once in its scope (§5.1).

struct Counter {
    int count;

    void add(int amount) {
        count += amount;
    }

    void add(int amount, int times) { // error
        count += amount * times;
    }
}

void main() {
    Counter counter;
}
