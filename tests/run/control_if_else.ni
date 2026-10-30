// if, else if and else, with blocks or single statements (§10, Appendix A).

String classify(int value) {
    if (value < 0) {
        return "negative";
    } else if (value == 0) {
        return "zero";
    } else {
        return "positive";
    }
}

void main() {
    println(classify(-5));          // expect: negative
    println(classify(0));           // expect: zero
    println(classify(8));           // expect: positive
    int x = 3;
    if (x > 2)
        println("single statement");    // expect: single statement
    if (x > 5)
        println("not printed");
    else
        println("else branch");         // expect: else branch
}
