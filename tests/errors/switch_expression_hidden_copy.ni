// A switch expression that picks an existing object can't be used where it would act on a copy (§10.1).

struct Counter {
    int value;

    void bump() {
        value = value + 1;
    }
}

void main() {
    Counter a = Counter();
    Counter b = Counter();
    boolean pickA = true;
    (switch (pickA) { case true -> a; default -> b; }).bump(); // error
}
