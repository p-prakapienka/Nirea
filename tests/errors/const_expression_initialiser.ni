// A constant's initialiser is a literal, a negated numeric literal or an enum
// value, not an expression (§5.5).

const int TOTAL = 2 + 3; // error

void main() {
    println("constants");
}
