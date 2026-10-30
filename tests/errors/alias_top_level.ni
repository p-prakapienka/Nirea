// Aliases can only be declared inside functions, never globally (§9, rules 1 and 3).

const int LIMIT = 3;

alias limit = LIMIT; // error

void main() {
    println(LIMIT);
}
