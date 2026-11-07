// Strings follow the same rules as structs: assignment, return and value
// parameters give the new owner its own copy; a literal always creates a new
// String (§6.5, §7.1).

const String APP_NAME = "Nirea";

struct Named {
    String name;

    String getName() {
        return name;
    }
}

int countWith(String text) {
    text.append("!");
    return text.length();
}

void main() {
    String first = APP_NAME;
    String second = first;
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
    named.name = first;
    String fromField = named.name;
    fromField.setCharAt(0, 'J');
    println(fromField);             // expect: Jirea
    println(named.getName());       // expect: Nirea
    println(countWith(named.name)); // expect: 6
    println(named.name);            // expect: Nirea
}
