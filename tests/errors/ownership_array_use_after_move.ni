// Arrays are owned values: assignment moves them (§6.6, §7.1).

void main() {
    int[3] values = {1, 2, 3};
    int[3] moved = values;
    println(values[0]); // error
}
