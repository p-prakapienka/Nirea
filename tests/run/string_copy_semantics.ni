// Strings copy like values: assignment, fields and returns copy, and the
// original stays usable (§6.5).

const String APP_NAME = "Nirea";

struct Named {
    String name;

    String getName() {
        return name;
    }
}

void main() {
    String first = APP_NAME;
    String second = first;
    second.append("!");
    println(first);                 // expect: Nirea
    println(second);                // expect: Nirea!

    Named named;
    named.name = first;
    String fromField = named.name;
    fromField.setCharAt(0, 'J');
    println(fromField);             // expect: Jirea
    println(named.name);            // expect: Nirea
    String returned = named.getName();
    returned.append("?");
    println(named.name);            // expect: Nirea
    println(returned);              // expect: Nirea?

    String[2] names = {first, second};
    String picked = names[1];
    picked.append("!");
    println(names[1]);              // expect: Nirea!
    println(picked);                // expect: Nirea!!

    boolean useFirst = true;
    String chosen = switch (useFirst) { case true -> first; default -> second; };
    chosen.append("*");
    println(first);                 // expect: Nirea
    println(chosen);                // expect: Nirea*
    println(switch (useFirst) { case true -> first; default -> second; }); // expect: Nirea
}
