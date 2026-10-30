// do-while loops run the body before testing the condition (§10).

void main() {
    int n = 0;
    do {
        println(n);
        n++;
    } while (n < 3);
    boolean again = false;
    do {
        println("runs once");
    } while (again);
}

// expect: 0
// expect: 1
// expect: 2
// expect: runs once
