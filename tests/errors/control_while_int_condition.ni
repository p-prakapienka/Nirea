// Loop conditions must have type boolean (§10).

void main() {
    int remaining = 3;
    while (remaining) { // error
        remaining--;
    }
}
