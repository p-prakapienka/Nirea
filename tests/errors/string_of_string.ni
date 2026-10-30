// String.of takes primitive or enum values; copy() copies a String (§6.5).

void main() {
    String a = "a";
    String b = String.of(a); // error
}
