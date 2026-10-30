// break outside a loop is an error (§10).

void main() {
    int x = 1;
    if (x == 1) {
        break; // error
    }
}
