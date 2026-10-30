// Built-in methods follow the same rule: a String can't be passed to its own
// append or equals; pass a copy (§9.3).

void main() {
    String text = "ab";
    text.append(text.copy());
    text.append(text); // error: this call already reaches text
    println(text.equals(text)); // error: this call already reaches text
}
