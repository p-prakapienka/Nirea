// An array variable declared without an initialiser has no elements until an
// array is assigned to it; T[N]() creates one (§6.6, §6.8).

void main() {
    int[3] created = int[3]();
    created[0] = 1;
    int[3] values;
    values[0] = 1; // error: it has no value yet
}
