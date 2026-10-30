// An array initialiser can be used only to initialise a variable declaration (§6.6).

void main() {
    int[3] values = {1, 2, 3};
    values = {4, 5, 6}; // error
}
