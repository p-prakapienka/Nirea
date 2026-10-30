// An enum value prints its own name, without the enum or namespace name (§15).

namespace Audio {
    enum State { PLAYING, PAUSED }
}

void main() {
    Audio.State state = Audio.State.PAUSED;
    println(state);           // expect: PAUSED
}
