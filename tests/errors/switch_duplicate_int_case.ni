// Duplicate cases are rejected, including a value repeated in a later label
// list (§10.1, §10.2). The switch is on one line so the diagnostic location
// is unambiguous.

String name(int value) {
    return switch (value) { case 1 -> "one"; case 2, 1 -> "again"; default -> "other"; }; // error
}

void main() {
    println(name(1));
}
