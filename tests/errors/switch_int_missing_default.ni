// Integer switches require a default branch (§10.2). The switch is on one
// line so the diagnostic location is unambiguous.

String name(int value) {
    return switch (value) { case 0 -> "zero"; case 1 -> "one"; }; // error
}

void main() {
    println(name(0));
}
