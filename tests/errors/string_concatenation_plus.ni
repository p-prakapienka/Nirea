// + doesn't concatenate strings (§6.5).

void main() {
    String a = "a";
    String b = "b";
    println(a + b); // error
}
