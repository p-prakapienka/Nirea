// Named constants aren't case values (§10.1).

const int LIMIT = 10;

void main() {
    int value = 10;
    switch (value) {
        case LIMIT -> println("limit"); // error
        default -> println("other");
    }
}
