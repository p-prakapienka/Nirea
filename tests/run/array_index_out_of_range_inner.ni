// Each index of a multi-dimensional array is checked against its own
// dimension (§6.6).
// runtime-error: index out of range

int cell(int[][]* grid, int row, int column) {
    return grid[row][column];
}

void main() {
    int[][] grid = int[2][3];
    grid[1][2] = 7;
    println(cell(grid, 1, 2));      // expect: 7
    println(cell(grid, 1, 3));
    println("not reached");
}
