// An array size is an integer literal or an integer constant, not a variable (§6.6).

void main() {
    int size = 3;
    int[size] values; // error
}
