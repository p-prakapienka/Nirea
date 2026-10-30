// Imports a file by its path from the source root (§12.2). From Main.ni both
// lookups find the same file, which is not ambiguous (§12.3).
import com/example/Audio.ni;

void main() {
    Audio.play();
    println(Audio.volumeLabel(3));
    println(Audio.volumeLabel(0));
}
