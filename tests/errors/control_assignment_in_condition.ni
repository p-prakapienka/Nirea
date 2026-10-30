// Assignment is a statement, not an expression (§10). The statement is on one
// line so the diagnostic location is unambiguous.

void main() {
    int value = 0;
    if ((value = 5) == 5) { println("assigned"); } // error
}
