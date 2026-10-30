// Members of different namespaces can share a name, and one namespace can use
// another's members through its name (§11).

namespace English {
    String greet() {
        return "Hello";
    }
}

namespace French {
    String greet() {
        return "Bonjour";
    }

    String borrowed() {
        return English.greet();
    }
}

void main() {
    println(English.greet());       // expect: Hello
    println(French.greet());        // expect: Bonjour
    println(French.borrowed());     // expect: Hello
}
