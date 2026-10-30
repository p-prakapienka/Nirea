// charAt outside the string is a runtime error (§6.5).

void main() {
    String text = "abc";
    println(text.charAt(3));
}
// runtime-error: index out of range
