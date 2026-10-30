// Each evaluation of a literal is a new String; changing one doesn't change the others (§6.5).

void main() {
    for (int i = 0; i < 3; i++) {
        String s = "ab";
        s.appendChar('c');
        println(s);                      // expect: abc
    }                                    // expect: abc
                                         // expect: abc
}
