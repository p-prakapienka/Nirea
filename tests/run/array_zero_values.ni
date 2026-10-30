// T[N]() creates an array with every element at its zero value (§6.6, §6.8).

enum Level {
    LOW,
    HIGH
}

struct Point {
    int x;
    int y;
}

void main() {
    int[4] numbers = int[4]();
    println(numbers[0]);            // expect: 0
    println(numbers[3]);            // expect: 0

    float64[2] reals = float64[2]();
    if (reals[1] == 0.0) {
        println("float zero");      // expect: float zero
    }
    boolean[2] flags = boolean[2]();
    if (!flags[0]) {
        println("boolean false");   // expect: boolean false
    }
    char[2] letters = char[2]();
    if (letters[1] == '\0') {
        println("char zero");       // expect: char zero
    }
    Level[3] levels = Level[3]();
    println(levels[2]);             // expect: LOW
    Point[2] points = Point[2]();
    println(points[1].y);           // expect: 0
    uint64[2][2] grid = uint64[2][2]();
    println(grid[1][1]);            // expect: 0
    println(String[3]()[2].length()); // expect: 0
}
