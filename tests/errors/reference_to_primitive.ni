// Only struct, array, String and interface types have references (§9).

void main() {
    int count = 1;
    int* reference = count; // error: only struct, array, String and interface types have references
}
