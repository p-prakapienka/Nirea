// Namespace members are accessed through the namespace name (§11).

namespace Audio {
    void play() {
        println("Playing audio");
    }

    void stop() {
        println("Stopping audio");
    }

    struct AudioEngine {
        int volume;

        void initialise() {
            volume = 5;
        }
    }
}

void main() {
    Audio.play();                   // expect: Playing audio
    Audio.AudioEngine engine;
    engine.initialise();
    println(engine.volume);         // expect: 5
    Audio.AudioEngine other = Audio.AudioEngine();
    println(other.volume);          // expect: 0
    Audio.stop();                   // expect: Stopping audio
}
