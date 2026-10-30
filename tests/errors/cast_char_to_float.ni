// char can be cast to and from integer types, but not floating-point types (§6.2).

void main() {
    char letter = 'A';
    float64 value = (float64) letter; // error
}
