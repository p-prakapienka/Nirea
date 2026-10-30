// The right operand of || runs only on some paths, so the local may still own its object (§7.1, §10).

struct Box {
    int x;
}

boolean consume(Box box) {
    return box.x > 0;
}

void main() {
    Box a;
    int k = 1;
    if (k > 0 || consume(a)) {
        println("yes");
    }
    println(a.x); // error: may have been given
}
