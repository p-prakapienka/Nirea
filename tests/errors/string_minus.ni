// Only `+` works on strings (§6.5).

void main() {
    String a = "ab";
    String b = a - "b"; // error
}
