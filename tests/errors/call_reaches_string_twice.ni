// Built-in methods with String* parameters follow the same rule: a String
// can't be passed to its own indexOf or equals; pass a copy (§9.3).

void main() {
    String text = "ab";
    println(text.indexOf(text.copy()));
    println(text.indexOf(text)); // error: this call already reaches text
    println(text.equals(text)); // error: this call already reaches text
}
