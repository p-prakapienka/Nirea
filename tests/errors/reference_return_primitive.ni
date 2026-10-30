// Only struct, array, String and interface types can be returned by reference
// (§9.2).

struct Counter {
    int value;

    int* get() { // error: only struct, array, String and interface types have references
        return value;
    }
}

void main() {
    println(1);
}
