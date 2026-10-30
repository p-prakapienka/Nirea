// Functions in extern "C" blocks can't return references (§9.2).

extern "C" {
    String* getenv(String* name); // error: can't return references
}

void main() {
    println(1);
}
