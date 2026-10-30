// Switch expressions produce values usable in declarations, assignments,
// arguments and other expressions (§10.1, §10.2).

void main() {
    int n = 2;
    int doubled = switch (n) {
        case 1 -> 10;
        case 2 -> 20;
        default -> 0;
    };
    println(doubled);
    println(1 + switch (n) {
        case 2 -> 4;
        default -> 0;
    });
    println(switch (n) {
        case 2 -> "two";
        default -> "other";
    });
    int total = 0;
    total = switch (n) {
        case 2 -> 7;
        default -> 1;
    };
    println(total);
}

// expect: 20
// expect: 5
// expect: two
// expect: 7
