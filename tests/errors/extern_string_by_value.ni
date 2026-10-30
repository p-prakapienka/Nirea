// C functions receive strings by reference (§14).

extern "C" {
    int puts(String text); // error
}

void main() {
    println(1);
}
