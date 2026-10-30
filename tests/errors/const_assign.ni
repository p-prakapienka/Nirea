// Constants aren't variables and can't be assigned (§5.5).

const int LIMIT = 10;

void main() {
    LIMIT = 20; // error
}
