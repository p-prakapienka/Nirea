// for loops in the Java and C form; each part of the header is optional
// (§10, Appendix A).

void main() {
    for (int i = 0; i < 3; i++) {
        println(i);
    }
    int j;
    for (j = 10; j > 7; j -= 1) {
        println(j);
    }
    println(j);
    int k = 0;
    for (; k < 2; k++) {
        println(k);
    }
    int n = 0;
    for (;;) {
        n++;
        if (n == 4) {
            break;
        }
    }
    println(n);
}

// expect: 0
// expect: 1
// expect: 2
// expect: 10
// expect: 9
// expect: 8
// expect: 7
// expect: 0
// expect: 1
// expect: 4
