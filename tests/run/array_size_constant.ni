// An array size can be an integer constant, including one in a namespace (§6.6).

const int SIZE = 3;

namespace Limits {
    const int CAPACITY = 2;
}

void main() {
    int[SIZE] values = int[SIZE]();
    println(values.length);         // expect: 3
    int[Limits.CAPACITY] more = int[Limits.CAPACITY]();
    println(more.length);           // expect: 2
    int[SIZE][Limits.CAPACITY] grid = int[SIZE][Limits.CAPACITY]();
    println(grid[2].length);        // expect: 2
}
