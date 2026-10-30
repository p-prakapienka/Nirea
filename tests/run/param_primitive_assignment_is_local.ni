// Primitive, enum and String parameters are local copies the function may assign (§7.3).

enum Level { LOW, HIGH }

void change(int number, String text, Level level) {
    number = number + 1;
    text = "changed";
    level = Level.HIGH;
    println(number);          // expect: 2
    println(text);            // expect: changed
    println(level);           // expect: HIGH
}

void main() {
    int number = 1;
    String text = "original";
    Level level = Level.LOW;
    change(number, text, level);
    println(number);          // expect: 1
    println(text);            // expect: original
    println(level);           // expect: LOW
}
