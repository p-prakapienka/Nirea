// Switch statements: an arm is a single statement or a block, and a case can
// list several values (§10.1).

enum State {
    IDLE,
    RUNNING,
    STOPPED
}

void report(State state) {
    switch (state) {
        case IDLE -> println("Idle");
        case RUNNING, STOPPED -> {
            println("Busy");
        }
    }
}

void main() {
    report(State.IDLE);             // expect: Idle
    report(State.RUNNING);          // expect: Busy
    report(State.STOPPED);          // expect: Busy
}
