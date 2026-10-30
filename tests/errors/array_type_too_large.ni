// Types too large to represent are rejected (§6.6).

void main() {
    int64[2147483647][2147483647] huge; // error
}
