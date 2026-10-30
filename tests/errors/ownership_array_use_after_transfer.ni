// Arrays are owned values: assignment transfers ownership of them (§6.6, §7.1).

void main() {
    int[3] values = {1, 2, 3};
    int[3] newOwner = values;
    println(values[0]); // error
}
