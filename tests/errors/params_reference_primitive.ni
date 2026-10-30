// Only struct, array and interface parameters can be references (§7.3).

void increment(int* value) { // error
    println(value);
}

void main() {
    increment(1);
}
