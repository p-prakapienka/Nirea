// The String arguments of built-in methods are values, so a String can be
// passed to its own methods; the argument doesn't change (§6.5, §7.3).

void main() {
    String s = "ab";
    s.append(s);
    s.append(s);
    println(s);                          // expect: abababab
    println(s.indexOf(s));               // expect: 0
    println(s.equals(s));                // expect: true

    String* reference = s;
    String other = "x";
    reference.append(reference);
    other.append(reference);
    println(s);                          // expect: abababababababab
    println(other);                      // expect: xabababababababab
}
