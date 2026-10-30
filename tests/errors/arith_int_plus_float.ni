// Neither int nor float converts to the other implicitly (§6.4).

void main() {
    int count = 3;
    float scale = 1.5;
    println(count * scale); // error
}
