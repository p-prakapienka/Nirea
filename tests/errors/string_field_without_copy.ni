// A String field stays part of its owner (§6.5, §7.1).

struct Named {
    String name;
}

void main() {
    Named named;
    String taken = named.name; // error
}
