// A String value parameter is a copy; a String* parameter changes the caller's string (§6.5, §7.3).

String shout(String text) {
    text.appendChar('!');
    return text;
}

void exclaim(String* text) {
    text.appendChar('!');
}

void main() {
    String hello = "hello";
    String loud = shout(hello.copy());
    println(hello);                      // expect: hello
    println(loud);                       // expect: hello!
    exclaim(hello);
    println(hello);                      // expect: hello!
    String moved = shout(hello);
    println(moved);                      // expect: hello!!
}
