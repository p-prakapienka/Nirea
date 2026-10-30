// Ping.ni and Pong.ni import each other; circular imports are allowed (§12.3).
import Pong.ni;

int ping(int n) {
    if (n == 0) {
        return 0;
    }
    return pong(n - 1) + 1;
}
