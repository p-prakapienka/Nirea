// A function can't assign to an array reference parameter as a whole (§7.3).

void clear(int[]* values) {
    int[] empty;
    values = empty; // error
}

void main() {
    int[] values = {1, 2, 3};
    clear(values);
}
