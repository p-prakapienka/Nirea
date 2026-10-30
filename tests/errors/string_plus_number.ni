// `+` concatenates two strings; other values are converted with String.of (§6.5).

void main() {
    int count = 3;
    String text = "count: " + count; // error: String.of
}
