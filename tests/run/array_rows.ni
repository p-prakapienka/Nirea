// T[a][b] creates a arrays of b elements. The rows are separate arrays that
// can be replaced with rows of other lengths (§6.6).

void main() {
    int[][] grid = int[2][3];
    grid[1][2] = 7;
    println(grid.length);            // expect: 2
    println(grid[0].length);         // expect: 3
    println(grid[1][2]);             // expect: 7
    grid[0] = int[5];
    println(grid[0].length);         // expect: 5
    int[][] jagged = {{1}, {2, 3}};
    println(jagged[1].length);       // expect: 2
}
