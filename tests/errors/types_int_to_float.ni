// int converts implicitly to int64 and float64 only, not to float (§6.2).

void main() {
    int count = 3;
    float ratio = count; // error
}
