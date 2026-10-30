// Character switches with a default branch (§10.1, §10.2).

String classify(char c) {
    return switch (c) {
        case 'a', 'e', 'i', 'o', 'u' -> "vowel";
        case ' ' -> "space";
        case '\n' -> "newline";
        default -> "other";
    };
}

void main() {
    println(classify('e'));         // expect: vowel
    println(classify(' '));         // expect: space
    println(classify('\n'));        // expect: newline
    println(classify('z'));         // expect: other
}
