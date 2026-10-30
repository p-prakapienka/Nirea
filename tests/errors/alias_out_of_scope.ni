// Aliases follow lexical block scope (§9, rule 2).

void main() {
    int[2] values = {1, 2};
    {
        alias first = values[0];
        println(first);
    }
    println(first); // error
}
