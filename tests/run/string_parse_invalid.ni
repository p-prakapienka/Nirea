// Text that isn't a number is a runtime error (§6.5).

void main() {
    String text = "12ab";
    println("before");                   // expect: before
    println(text.toInt());
}
// runtime-error: invalid number
