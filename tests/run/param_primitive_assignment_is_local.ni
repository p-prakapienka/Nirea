// Primitive and enum parameters are local copies the function may assign (§7.3).

enum Level { LOW, HIGH }

void change(int number, Level level) {
    number = number + 1;
    level = Level.HIGH;
    println(number);          // expect: 2
    println(level);           // expect: HIGH
}

void main() {
    int number = 1;
    Level level = Level.LOW;
    change(number, level);
    println(number);          // expect: 1
    println(level);           // expect: LOW
}
