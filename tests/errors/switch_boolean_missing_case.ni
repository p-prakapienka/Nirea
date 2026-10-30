// Boolean switches must cover both values unless a default exists (§10.2).
// The switch is on one line so the diagnostic location is unambiguous.

String describe(boolean flag) {
    return switch (flag) { case true -> "on"; }; // error
}

void main() {
    println(describe(true));
}
