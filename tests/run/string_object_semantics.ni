// Strings are owned objects: assignment transfers ownership, copy() makes an
// independent String, and a String field stays part of its owner (§6.5, §7).

const String APP_NAME = "Nirea";

struct Named {
    String name;
}

void show(String* text) {
    println(text);
}

void main() {
    String first = APP_NAME;
    String second = first.copy();
    second.append("!");
    println(first);                 // expect: Nirea
    println(second);                // expect: Nirea!
    show(first);                    // expect: Nirea

    String third = second;          // second is no longer accessible
    println(third);                 // expect: Nirea!
    second = "reused";
    println(second);                // expect: reused

    Named named;
    named.name = first;             // the field becomes the owner
    String fromField = named.name.copy();
    fromField.setCharAt(0, 'J');
    println(fromField);             // expect: Jirea
    println(named.name);            // expect: Nirea
    println(APP_NAME);              // expect: Nirea
}
