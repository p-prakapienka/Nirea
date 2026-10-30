// Character switches require a default branch (§10.2). The switch is on one
// line so the diagnostic location is unambiguous.

String kind(char c) {
    return switch (c) { case 'a', 'e', 'i', 'o', 'u' -> "vowel"; case ' ' -> "space"; }; // error
}

void main() {
    println(kind('a'));
}
