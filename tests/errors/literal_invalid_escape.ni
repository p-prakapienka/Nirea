// Unknown escape sequences are errors (§6.3).

void main() {
    println("bad \q escape"); // error
}
