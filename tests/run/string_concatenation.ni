// `a + b` creates a new String; `a += b` appends (§6.5).

struct Person {
    String name;
}

String greet(String* name) {
    return "Hello, " + name + "!";
}

void main() {
    String name = "Nirea";
    String greeting = "Hello, " + name + "!";
    println(greeting);                       // expect: Hello, Nirea!
    println(name);                           // expect: Nirea
    greeting += " v" + String.of(8);
    println(greeting);                       // expect: Hello, Nirea! v8
    println(greet(name));                    // expect: Hello, Nirea!

    Person person;
    person.name = "Ada";
    person.name += " Lovelace";
    println(person.name);                    // expect: Ada Lovelace
    println(person.name + "" == "Ada Lovelace"); // expect: true

    String twice = name;
    twice += twice;
    println(twice);                          // expect: NireaNirea

    String built;
    for (int i = 0; i < 3; i++) {
        built = built + String.of(i);
    }
    println(built);                          // expect: 012
}
