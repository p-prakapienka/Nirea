// Duplicate cases are rejected (§10.2). The switch is on one line so the
// diagnostic location is unambiguous.

enum State {
    IDLE,
    RUNNING,
    STOPPED
}

void report(State state) {
    switch (state) { case IDLE -> println("Idle"); case RUNNING, STOPPED -> println("Busy"); case IDLE -> println("Again"); } // error
}

void main() {
    report(State.IDLE);
}
