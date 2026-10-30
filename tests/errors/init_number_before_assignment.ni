// Numbers have no implicit value either: ++, -- and compound assignment use
// the variable (§6.8).

void main() {
    int count;
    count++; // error: it has no value yet
    int total;
    total += 5; // error: it has no value yet
}
