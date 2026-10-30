// A number out of range for the type is a runtime error (§6.5).

void main() {
    String text = "3000000000";
    println(text.toInt());
}
// runtime-error: invalid number
