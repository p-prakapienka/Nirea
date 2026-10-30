// Enums support == and != only (§6.7).

enum Level {
    LOW,
    HIGH
}

void main() {
    Level level = Level.LOW;
    if (level < Level.HIGH) { // error
        println("lower");
    }
}
