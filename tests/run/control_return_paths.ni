// Infinite loops and switch statements whose branches all return count as returning (§10).

enum Sign { NEGATIVE, ZERO, POSITIVE }

int firstAbove(int limit) {
    int i = 0;
    while (true) {
        if (i * i > limit) {
            return i;
        }
        i++;
    }
}

int classify(Sign sign) {
    switch (sign) {
        case NEGATIVE -> {
            return -1;
        }
        case ZERO -> {
            return 0;
        }
        case POSITIVE -> {
            return 1;
        }
    }
}

int early(int x) {
    return x;
    println("never printed");
}

void main() {
    println(firstAbove(10));            // expect: 4
    println(classify(Sign.POSITIVE));   // expect: 1
    println(early(7));                  // expect: 7
}
