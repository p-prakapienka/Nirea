// Case labels use unqualified enum values (§10.1).

enum State { IDLE, RUNNING }

void main() {
    State state = State.IDLE;
    switch (state) {
        case State.IDLE -> println("idle"); // error
        case RUNNING -> println("running");
    }
}
