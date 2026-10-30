// Objects nothing owns are destroyed at the end of their statement (§7.6).

String make(int n) {
    return String.of(n);
}

int count(String* text) {
    return text.length();
}

void main() {
    make(1);
    println(make(22));                   // expect: 22
    println(count(make(333)));           // expect: 3
    println(make(4444).length());        // expect: 4
    boolean big = make(5).length() > 0 && make(66).length() == 2;
    println(big);                        // expect: true
    String chosen = switch (big) { case true -> make(7); default -> "no"; };
    println(chosen);                     // expect: 7
    println(String.of(8) == "8");        // expect: true
    if (make(9) == "9") {
        println("nine");                 // expect: nine
    }
    int i = 0;
    while (make(i) != "3") {
        i++;
    }
    println(i);                          // expect: 3
}
