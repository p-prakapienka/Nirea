// while loops test the condition before each iteration (§10).

void main() {
    int countdown = 3;
    while (countdown > 0) {
        println(countdown);
        countdown--;
    }
    boolean running = false;
    while (running) {
        println("never");
    }
    println("liftoff");
}

// expect: 3
// expect: 2
// expect: 1
// expect: liftoff
