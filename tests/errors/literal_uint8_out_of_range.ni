// A literal that doesn't fit its target type is a compile-time failure (§6.3).

void main() {
    uint8 level = 256; // error
}
