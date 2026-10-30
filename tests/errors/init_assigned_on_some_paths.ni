// A variable must be assigned on every path that reaches a use (§6.8).

void main() {
    boolean ready = true;
    String status;
    if (ready) {
        status = "ready";
    }
    println(status); // error: it may have no value on some path
}
