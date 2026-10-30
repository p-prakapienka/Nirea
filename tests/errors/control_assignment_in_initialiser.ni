// Assignment is a statement, so assignments can't be chained (§10).

void main() {
    int first = 0;
    int second = first = 3; // error
}
