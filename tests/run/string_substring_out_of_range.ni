// substring needs 0 <= from <= to <= length() (§6.5).

void main() {
    String text = "abc";
    println(text.substring(2, 1));
}
// runtime-error: index out of range
