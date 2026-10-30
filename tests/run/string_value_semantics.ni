// Strings aren't owned: assigning or passing a String copies it, and reading
// a String field doesn't transfer it (§6.5, §7).

const String APP_NAME = "Nirea";

struct Named {
    String name;
}

void show(String text) {
    println(text);
}

void main() {
    String first = APP_NAME;
    String second = first;
    println(first);                 // expect: Nirea
    println(second);                // expect: Nirea
    show(first);                    // expect: Nirea
    second = "changed";
    println(first);                 // expect: Nirea
    println(second);                // expect: changed

    Named named;
    named.name = first;
    String fromField = named.name;
    println(fromField);             // expect: Nirea
    println(named.name);            // expect: Nirea
}
