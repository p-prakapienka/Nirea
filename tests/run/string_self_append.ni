// += can append a String to itself; it adds the bytes the String had before (§6.5).

void main() {
    String s = "ab";
    s += s;
    s += s;
    println(s);                          // expect: abababab
}
