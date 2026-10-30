// util/Text.ni doesn't exist next to this file, so it resolves against the
// source root (§12.3).
import util/Text.ni;

namespace Audio {
    void play() {
        println("Playing audio");
    }

    String volumeLabel(int level) {
        return Text.levelName(level);
    }
}
