// break and continue inside a switch arm apply to the enclosing loop (§10.1).

enum Step {
    GO,
    SKIP,
    STOP
}

Step stepFor(int n) {
    if (n == 2) {
        return Step.SKIP;
    }
    if (n == 4) {
        return Step.STOP;
    }
    return Step.GO;
}

void main() {
    for (int i = 0; i < 10; i++) {
        switch (i) {
            case 1, 3 -> continue;
            case 5 -> break;
            default -> print(i);
        }
        println(" after switch");
    }

    int n = 0;
    while (n < 10) {
        n++;
        switch (stepFor(n)) {
            case GO -> {
                println(n);
            }
            case SKIP -> {
                continue;
            }
            case STOP -> {
                break;
            }
        }
    }
    println(n);
}

// expect: 0 after switch
// expect: 2 after switch
// expect: 4 after switch
// expect: 1
// expect: 3
// expect: 4
