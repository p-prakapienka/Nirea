// Conditions must have type boolean; numbers don't convert to boolean (§10).

void main() {
    int count = 1;
    if (count) { // error
        println("non-zero");
    }
}
