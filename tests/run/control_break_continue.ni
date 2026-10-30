// break and continue in for, while and do-while loops; in nested loops they
// apply to the innermost loop (§10).

void main() {
    for (int i = 0; i < 6; i++) {
        if (i % 2 == 0) {
            continue;
        }
        println(i);
    }

    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 5; inner++) {
            if (inner == 2) {
                break;
            }
            print(outer);
            println(inner);
        }
    }

    int n = 0;
    while (true) {
        n++;
        if (n < 3) {
            continue;
        }
        break;
    }
    println(n);

    int m = 0;
    do {
        m++;
        if (m == 2) {
            continue;
        }
        println(m);
    } while (m < 3);
}

// expect: 1
// expect: 3
// expect: 5
// expect: 00
// expect: 01
// expect: 10
// expect: 11
// expect: 3
// expect: 1
// expect: 3
