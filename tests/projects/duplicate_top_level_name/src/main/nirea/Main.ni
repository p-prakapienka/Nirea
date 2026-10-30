// Every .ni file under the source root is part of the program, and top-level
// names must be unique across it (§12.3). extra/Unused.ni is never imported
// but still declares a second helper.

int helper() {
    return 1;
}

void main() {
    println(helper());
}
