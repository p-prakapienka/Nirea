// The zero value of String is "" (§6.8).

struct Label {
    String text;
}

void main() {
    Label label = Label();
    if (label.text == "") {
        println("field is empty");      // expect: field is empty
    }
    print("[");
    print(label.text);
    println("]");                       // expect: []

    String[] names = String[2];
    if (names[1] == "") {
        println("element is empty");    // expect: element is empty
    }
}
