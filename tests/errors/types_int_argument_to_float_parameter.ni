// The implicit conversion rules apply to arguments too: int doesn't convert
// to float (§6.2).

float identity(float value) {
    return value;
}

void main() {
    int count = 3;
    float result = identity(count); // error
}
