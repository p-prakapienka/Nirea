// A field can be named copy (§7.2).

struct Tally {
    int copy;
}

void main() {
    Tally tally = Tally();
    tally.copy = 3;
    Tally other = tally;
    println(other.copy);      // expect: 3
}
