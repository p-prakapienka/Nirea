// append takes its argument by value, so a String can be appended to itself;
// the argument doesn't change (§6.5, §7.3).

void main() {
    String s = "ab";
    s.append(s);
    s.append(s);
    println(s);                          // expect: abababab

    String* reference = s;
    String other = "x";
    reference.append(reference);
    other.append(reference);
    println(s);                          // expect: abababababababab
    println(other);                      // expect: xabababababababab
}
