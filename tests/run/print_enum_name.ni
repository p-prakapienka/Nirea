// Printing an enum value prints its name (§6.7).

enum Color {
    RED,
    GREEN,
    BLUE
}

void main() {
    Color color = Color.GREEN;
    println(color);                 // expect: GREEN
    println(Color.BLUE);            // expect: BLUE
    print(Color.RED);
    println(Color.RED);             // expect: REDRED
}
