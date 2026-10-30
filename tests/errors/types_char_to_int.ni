// char never converts implicitly; a conversion to an integer type needs a cast (§6.2).

void main() {
    char letter = 'A';
    int code = letter; // error
}
