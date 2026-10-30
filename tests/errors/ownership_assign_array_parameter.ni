// A function can't assign to an array reference parameter as a whole (§7.3).

void clear(int[3]* values) {
    int[3] empty;
    values = empty; // error
}

void main() {
    int[3] values = {1, 2, 3};
    clear(values);
}
