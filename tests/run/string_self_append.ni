// A String can be appended to itself (§6.5).

void main() {
    String s = "ab";
    s.append(s);
    s.append(s);
    println(s);                          // expect: abababab
}
