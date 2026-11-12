// String.of takes primitive or enum values; assigning a String copies it (§6.5).

void main() {
    String a = "a";
    String b = String.of(a); // error
}
