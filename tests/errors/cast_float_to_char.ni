// Floating-point values can't be cast to char (§6.2).

void main() {
    float64 value = 65.0;
    char c = (char) value; // error
}
