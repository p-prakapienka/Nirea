// Unary minus isn't allowed on unsigned types (§6.4).

void main() {
    uint value = 5;
    uint negated = -value; // error
}
