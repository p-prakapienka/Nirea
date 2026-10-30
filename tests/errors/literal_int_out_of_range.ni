// A literal that doesn't fit its target type is a compile-time failure; int
// is 32 bits (§6.2, §6.3).

void main() {
    int big = 2147483648; // error
}
