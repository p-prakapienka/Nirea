// Each namespace is declared once in a program (§11).

namespace Audio {
    void play() {
        println("play");
    }
}

namespace Audio { // error
    void stop() {
        println("stop");
    }
}

void main() {
    Audio.play();
}
