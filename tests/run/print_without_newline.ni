// print writes one value without a newline. println() with no argument ends
// the current line, or prints an empty line on its own (§15).

void main() {
    print("a");
    print(1);
    print("b");
    println("c");                   // expect: a1bc
    print("no newline yet");
    println();                      // expect: no newline yet
    print(-7);
    print(" ");
    println("done");                // expect: -7 done
    println();                      // expect:
    println("end");                 // expect: end
}
