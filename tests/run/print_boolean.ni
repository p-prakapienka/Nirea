// Booleans print as their literal names, true and false (§6.3, §15).

void main() {
    boolean yes = true;
    boolean no;
    println(yes);                   // expect: true
    println(no);                    // expect: false
    println(1 < 2);                 // expect: true
    print(false);
    println(true);                  // expect: falsetrue
}
