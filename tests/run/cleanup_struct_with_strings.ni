// Objects that own strings are destroyed with their owner; replaced objects are
// destroyed on assignment; copies are deep (§7.2, §7.5, §7.6).

struct Person {
    String name;
    String[2] tags;

    Person(String name) {
        this.name = name;
        tags[0] = "a";
        tags[1] = "b";
    }
}

Person make(int n) {
    Person p = Person(String.of(n));
    p.tags[1].append(String.of(n));
    return p;
}

void main() {
    Person first = make(1);
    Person second = first.copy();
    second.name.append("x");
    println(first.name);                 // expect: 1
    println(second.name);                // expect: 1x
    first = make(2);
    println(first.tags[1]);              // expect: b2
    Person[2] people = {make(3), make(4)};
    people[0] = make(5);
    println(people[0].name);             // expect: 5
    println(make(6).name);               // expect: 6
    first.name = "replaced";
    println(first.name);                 // expect: replaced
}
