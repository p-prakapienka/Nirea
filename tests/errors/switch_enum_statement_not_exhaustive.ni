// Switch statements must be exhaustive too (§10.2). The switch is on one line
// so the diagnostic location is unambiguous.

enum State {
    IDLE,
    RUNNING,
    STOPPED
}

void report(State state) {
    switch (state) { case IDLE -> println("Idle"); case RUNNING -> println("Running"); } // error
}

void main() {
    report(State.IDLE);
}
