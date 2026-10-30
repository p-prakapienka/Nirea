// Importing a file makes its declarations available; namespace access stays
// explicit (§11, §12.3).
import audio/Audio.ni;
import video/Video.ni;

struct Speaker : Audio.Playable {
    int volume;

    void play() {
        print("speaker at ");
        println(volume);
    }
}

void main() {
    Audio.Track track = Audio.makeTrack(3);
    println(track.length);
    println(Audio.DEFAULT_VOLUME);
    if (track.kind == Audio.Kind.SONG) {
        println("song");
    }
    Speaker speaker;
    speaker.volume = Audio.DEFAULT_VOLUME;
    Audio.playTwice(speaker);
    Video.show();
}
