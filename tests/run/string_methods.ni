// Built-in String methods (§6.5).

void main() {
    String text = "Hello";
    println(text.length());              // expect: 5
    println(text.charAt(1));             // expect: e
    text.setCharAt(0, 'J');
    println(text);                       // expect: Jello
    text.append(", world");
    text.appendChar('!');
    println(text);                       // expect: Jello, world!
    println(text.substring(7, 12));      // expect: world
    println(text.indexOf("world"));      // expect: 7
    println(text.indexOf("moon"));       // expect: -1
    println(text.indexOf(""));           // expect: 0
    String empty = "";
    println(empty.length());             // expect: 0
    empty.appendChar('x');
    println(empty);                      // expect: x
}
