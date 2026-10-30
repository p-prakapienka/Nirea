// A continue that skips the return leaves a path without a return value (§10).

int f(boolean c) { do { if (c) continue; return 1; } while (false); } // error

void main() {
    println(f(false));
}
