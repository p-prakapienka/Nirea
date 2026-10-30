// Every enum value must be covered when there is no default (§10.2). The
// switch is on one line so the diagnostic location is unambiguous.

enum State {
    IDLE,
    RUNNING,
    STOPPED
}

String describe(State state) {
    return switch (state) { case IDLE -> "Idle"; case RUNNING -> "Running"; }; // error
}

void main() {
    println(describe(State.IDLE));
}
