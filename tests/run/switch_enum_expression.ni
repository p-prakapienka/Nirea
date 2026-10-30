// A switch expression covering every enum value needs no default (§10.1, §10.2).

enum State {
    IDLE,
    RUNNING,
    STOPPED
}

String describe(State state) {
    return switch (state) {
        case IDLE -> "Idle";
        case RUNNING -> "Running";
        case STOPPED -> "Stopped";
    };
}

void main() {
    println(describe(State.IDLE));          // expect: Idle
    println(describe(State.RUNNING));       // expect: Running
    println(describe(State.STOPPED));       // expect: Stopped
}
