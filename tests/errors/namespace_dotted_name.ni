// A namespace has exactly one identifier; dots are not permitted (§11). The
// declaration is on one line so the diagnostic location is unambiguous.

namespace Audio.Core { void play() { println("play"); } } // error

void main() {
    println("dotted");
}
