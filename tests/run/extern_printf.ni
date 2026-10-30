// Calling C's printf through an extern "C" block, with a variadic argument
// list of primitives and strings (§14).

extern "C" {
    int printf(String* format, ...);
}

void main() {
    printf("Hello from Nirea!\n");          // expect: Hello from Nirea!
    printf("%d\n", 42);                     // expect: 42
    printf("%s and %d\n", "text", 7);       // expect: text and 7
    int value = -15;
    printf("value=%d\n", value);            // expect: value=-15
    String name = "Nirea";
    printf("name=%s\n", name);              // expect: name=Nirea
    int written = printf("abc\n");          // expect: abc
    printf("%d\n", written);                // expect: 4
}
