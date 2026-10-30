// A namespace member may share a top-level name; inside the namespace it takes precedence (§11).

String name() {
    return "top level";
}

namespace Audio {
    String name() {
        return "audio";
    }

    void show() {
        println(name());
    }
}

void main() {
    println(name());          // expect: top level
    println(Audio.name());    // expect: audio
    Audio.show();             // expect: audio
}
