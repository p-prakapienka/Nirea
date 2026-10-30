// Arrays are owned values like structs: assignment moves them and copy()
// makes an independent deep copy (§6.6, §7.1, §7.2).

struct Cell {
    int value;
}

void main() {
    int[3] first = {1, 2, 3};
    int[3] second = first;
    println(second[2]);             // expect: 3
    first = second.copy();
    first[0] = 10;
    println(first[0]);              // expect: 10
    println(second[0]);             // expect: 1

    Cell[2] original;
    original[0].value = 1;
    Cell[2] duplicate = original.copy();
    duplicate[0].value = 2;
    println(original[0].value);     // expect: 1
    println(duplicate[0].value);    // expect: 2

    int[2][2] grid = {{1, 2}, {3, 4}};
    int[2] row = grid[1].copy();
    row[0] = 30;
    println(grid[1][0]);            // expect: 3
    println(row[0]);                // expect: 30
}
