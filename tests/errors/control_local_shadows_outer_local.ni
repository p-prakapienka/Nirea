// A local variable can't reuse the name of an enclosing block's variable (§10).

void main() {
    int count = 1;
    if (count > 0) {
        int count = 2; // error
        println(count);
    }
}
