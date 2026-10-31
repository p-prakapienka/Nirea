// Array reference parameters (T[N]*) are non-owning: the function can write
// elements and the caller sees the changes (§6.6, §7.3).

struct Counter {
    int count;
}

void fill(int[]* values, int start) {
    for (int i = 0; i < values.length; i++) {
        values[i] = start + i;
    }
}

int sum(int[]* values) {
    int total = 0;
    for (int i = 0; i < values.length; i++) {
        total += values[i];
    }
    return total;
}

void bumpAll(Counter[]* counters) {
    counters[0].count++;
    counters[1].count += 10;
}

void markCorner(int[][]* grid) {
    grid[1][1] = 4;
}

void main() {
    int[] numbers = int[3];
    fill(numbers, 5);
    println(numbers[0]);            // expect: 5
    println(numbers[2]);            // expect: 7
    println(sum(numbers));          // expect: 18

    Counter[] counters = Counter[2];
    bumpAll(counters);
    bumpAll(counters);
    println(counters[0].count);     // expect: 2
    println(counters[1].count);     // expect: 20

    int[][] grid = int[2][2];
    markCorner(grid);
    println(grid[1][1]);            // expect: 4

    fill(numbers, 0);
    println(numbers[1]);            // expect: 1
}
