// Array elements start at their zero value (§6.6, §6.8).

enum Level {
    LOW,
    HIGH
}

struct Point {
    int x;
    int y;
}

void main() {
    int[4] numbers;
    println(numbers[0]);            // expect: 0
    println(numbers[3]);            // expect: 0

    float64[2] reals;
    if (reals[1] == 0.0) {
        println("float zero");      // expect: float zero
    }
    boolean[2] flags;
    if (!flags[0]) {
        println("boolean false");   // expect: boolean false
    }
    char[2] letters;
    if (letters[1] == '\0') {
        println("char zero");       // expect: char zero
    }
    Level[3] levels;
    println(levels[2]);             // expect: LOW
    Point[2] points;
    println(points[1].y);           // expect: 0
    uint64[2][2] grid;
    println(grid[1][1]);            // expect: 0
}
