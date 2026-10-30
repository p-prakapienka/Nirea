// Outside case labels, enum values are written qualified with the enum name (§6.7).

enum State {
    IDLE,
    RUNNING
}

void main() {
    State state = IDLE; // error
}
