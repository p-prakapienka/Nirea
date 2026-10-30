// T[A][B] is an array of A elements of type T[B] (§6.6).

void main() {
    int[2][3] grid;
    grid[1][2] = 7;
    println(grid[1][2]);            // expect: 7
    println(grid[0][2]);            // expect: 0
    println(grid.length);           // expect: 2
    println(grid[0].length);        // expect: 3

    int[2][2] matrix = {{1, 2}, {3, 4}};
    int sum = 0;
    for (int row = 0; row < matrix.length; row++) {
        for (int column = 0; column < matrix[row].length; column++) {
            sum += matrix[row][column];
        }
    }
    println(sum);                   // expect: 10

    int[2][2][2] cube;
    cube[1][0][1] = 5;
    println(cube[1][0][1]);         // expect: 5
    println(cube[1][0].length);     // expect: 2
}
