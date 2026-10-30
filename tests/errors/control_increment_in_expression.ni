// ++ and -- are statements and can't be nested inside other expressions (§10).

void main() {
    int count = 0;
    println(count++); // error
}
