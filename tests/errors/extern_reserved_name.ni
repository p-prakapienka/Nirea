// extern "C" functions can't use names reserved in C or by the compiler (§14).

extern "C" {
    int nr_t1(int x); // error
}

void main() {
    println(1);
}
