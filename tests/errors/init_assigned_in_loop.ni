// An assignment inside a loop body may never run, so it doesn't count after
// the loop (§6.8).

void main() {
    int last;
    for (int i = 0; i < 3; i++) {
        last = i;
    }
    println(last); // error: it may have no value on some path
}
