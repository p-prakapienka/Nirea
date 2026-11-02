// An array can't have more than 2147483647 elements; all sizes are checked
// before any array is created (§6.4, §6.6).

void main() {
    int64 rows = 3000000000;
    int[][] grid = int[2][rows]; // runtime-error: array length above 2147483647
}
