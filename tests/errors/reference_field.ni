// A struct field can't be a reference; references in fields are an open
// question (§7.3, §18).

struct Engine {
    int power;
}

struct Vehicle {
    Engine* engine; // error: fields can't be references
}

void main() {
    println(1);
}
