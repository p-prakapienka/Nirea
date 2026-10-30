// void can't be returned by reference (§9.2).

void* nothing() { // error: void can't be returned by reference
    return;
}

void main() {
    println(1);
}
