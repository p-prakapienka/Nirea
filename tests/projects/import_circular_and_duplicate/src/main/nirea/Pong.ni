import Ping.ni;

int pong(int n) {
    if (n == 0) {
        return 100;
    }
    return ping(n - 1) + 10;
}
