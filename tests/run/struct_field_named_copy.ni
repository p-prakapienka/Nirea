// Only a method can't be named copy; a field can (§7.2).

struct Tally {
    int copy;
}

void main() {
    Tally tally = Tally();
    tally.copy = 3;
    Tally other = tally.copy();
    println(other.copy);      // expect: 3
}
