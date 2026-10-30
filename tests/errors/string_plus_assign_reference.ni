// `+=` assigns, so it can't be used on a String reference as a whole; use append (§6.5).

void shout(String* text) {
    text += "!"; // error: append
}

void main() {
    String text = "hi";
    shout(text);
}
