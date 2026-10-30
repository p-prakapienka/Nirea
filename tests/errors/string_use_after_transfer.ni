// Assigning a String transfers ownership (§6.5, §7.1).

void main() {
    String first = "a";
    String second = first;
    println(first); // error
}
