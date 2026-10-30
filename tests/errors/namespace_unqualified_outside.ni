// Outside its namespace, a member is accessed through the namespace name (§11).

namespace Audio {
    void play() {
        println("play");
    }
}

void main() {
    play(); // error
}
