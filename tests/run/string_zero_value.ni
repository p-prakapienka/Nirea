// The zero value of String is "" (§6.8).

struct Label {
    String text;
}

void main() {
    String empty;
    if (empty == "") {
        println("local is empty");      // expect: local is empty
    }
    print("[");
    print(empty);
    println("]");                       // expect: []

    Label label;
    if (label.text == "") {
        println("field is empty");      // expect: field is empty
    }

    String[2] names;
    if (names[1] == "") {
        println("element is empty");    // expect: element is empty
    }
}
