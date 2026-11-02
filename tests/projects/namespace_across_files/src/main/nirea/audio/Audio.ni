namespace Audio {
    const int DEFAULT_VOLUME = 11;

    enum Kind {
        SONG,
        PODCAST
    }

    struct Track {
        int length;
        Kind kind;
    }

    interface Playable {
        void play();
    }

    Track makeTrack(int length) {
        Track track = Track();
        track.length = length;
        track.kind = Kind.SONG;
        return track;
    }

    void playTwice(Playable* playable) {
        playable.play();
        playable.play();
    }
}
