// Arrays are owned objects like structs: assigning one gives the new owner
// its own deep copy (§6.6, §7.1).

struct Cell {
    int value;
}

void main() {
    int[] first = {1, 2, 3};
    int[] second = first;
    println(second[2]);             // expect: 3
    first = second;
    first[0] = 10;
    println(first[0]);              // expect: 10
    println(second[0]);             // expect: 1

    Cell[] original = Cell[2];
    original[0].value = 1;
    Cell[] duplicate = original;
    duplicate[0].value = 2;
    println(original[0].value);     // expect: 1
    println(duplicate[0].value);    // expect: 2

    int[][] grid = {{1, 2}, {3, 4}};
    int[] row = grid[1];
    row[0] = 30;
    println(grid[1][0]);            // expect: 3
    println(row[0]);                // expect: 30
}
