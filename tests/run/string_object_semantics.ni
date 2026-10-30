// Strings follow the same rules as structs: assignment and return transfer
// ownership, copy() keeps both usable, value parameters receive copies; a
// literal always creates a new String (§6.5, §7).

const String APP_NAME = "Nirea";

struct Named {
    String name;

    String getName() {
        return name.copy();
    }
}

int countWith(String text) {
    text.append("!");
    return text.length();
}

void main() {
    String first = APP_NAME;
    String second = first.copy();
    second.append("!");
    println(first);                 // expect: Nirea
    println(second);                // expect: Nirea!

    String third = second;          // second is no longer accessible
    println(third);                 // expect: Nirea!
    second = "reused";              // a new object makes it usable again
    println(second);                // expect: reused

    println(countWith(first));      // expect: 6
    println(first);                 // expect: Nirea

    Named named = Named();
    named.name = first.copy();
    String fromField = named.name.copy();
    fromField.setCharAt(0, 'J');
    println(fromField);             // expect: Jirea
    println(named.getName());       // expect: Nirea
    println(countWith(named.name)); // expect: 6
    println(named.name);            // expect: Nirea
}
