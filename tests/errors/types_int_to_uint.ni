// There are no implicit signed-to-unsigned conversions (§6.2).

void main() {
    int count = 3;
    uint unsignedCount = count; // error
}
