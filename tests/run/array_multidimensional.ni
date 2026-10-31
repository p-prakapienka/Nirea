// T[][] is an array of A elements of type T[B] (§6.6).

void main() {
    int[][] grid = int[2][3];
    grid[1][2] = 7;
    println(grid[1][2]);            // expect: 7
    println(grid[0][2]);            // expect: 0
    println(grid.length);           // expect: 2
    println(grid[0].length);        // expect: 3

    int[][] matrix = {{1, 2}, {3, 4}};
    int sum = 0;
    for (int row = 0; row < matrix.length; row++) {
        for (int column = 0; column < matrix[row].length; column++) {
            sum += matrix[row][column];
        }
    }
    println(sum);                   // expect: 10

    int[][][] cube = int[2][2][2];
    cube[1][0][1] = 5;
    println(cube[1][0][1]);         // expect: 5
    println(cube[1][0].length);     // expect: 2
}
