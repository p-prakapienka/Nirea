// == and != compare string contents (§6.5).

const String GREETING = "hello";

String pick(int choice) {
    return switch (choice) {
        case 1 -> "hello";
        default -> "other";
    };
}

void main() {
    String word = "hello";
    if (word == "hello") {
        println("literal equal");       // expect: literal equal
    }
    if (word == GREETING) {
        println("constant equal");      // expect: constant equal
    }
    if (pick(1) == word) {
        println("returned equal");      // expect: returned equal
    }
    if (word == "\x68\x65llo") {
        println("escaped equal");       // expect: escaped equal
    }
    if (word != "help") {
        println("different");           // expect: different
    }
    if (word != "hell") {
        println("prefix differs");      // expect: prefix differs
    }
    if (word != "hello!") {
        println("longer differs");      // expect: longer differs
    }
    if (pick(2) != word) {
        println("other differs");       // expect: other differs
    }
}
