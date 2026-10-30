// There are no implicit narrowing conversions (§6.2).

void main() {
    int64 big = 5;
    int small = big; // error
}
