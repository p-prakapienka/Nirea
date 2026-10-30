// int64 converts implicitly to nothing else, not even float64 (§6.2).

void main() {
    int64 count = 5;
    float64 ratio = count; // error
}
