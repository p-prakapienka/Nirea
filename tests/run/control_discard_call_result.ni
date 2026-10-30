// A call's result may be discarded (§10).

int noisy() {
    println("called");
    return 42;
}

void main() {
    noisy();                  // expect: called
}
