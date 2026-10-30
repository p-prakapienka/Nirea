// Escape sequences in character and string literals (§6.3).

void main() {
    println((int) '\n');            // expect: 10
    println((int) '\t');            // expect: 9
    println((int) '\r');            // expect: 13
    println((int) '\0');            // expect: 0
    println((int) '\\');            // expect: 92
    println((int) '\'');            // expect: 39
    println((int) '\"');            // expect: 34
    println((int) '\x41');          // expect: 65
    println((int) '\x7F');          // expect: 127

    println("say \"hi\"");          // expect: say "hi"
    println("back\\slash");         // expect: back\slash
    println("it\'s");               // expect: it's
    println("\x4E\x69rea");         // expect: Nirea
    println("first\nsecond");       // expect: first
                                    // expect: second
}
