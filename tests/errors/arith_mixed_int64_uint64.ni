// Mixing int64 with uint64 needs a cast (§6.4).

void main() {
    int64 a = 5;
    uint64 b = 3;
    int64 sum = a + b; // error
}
