// Array reference parameters (T[N]*) are non-owning: the function can write
// elements and the caller sees the changes (§6.6, §7.3).

struct Counter {
    int count;
}

void fill(int[3]* values, int start) {
    for (int i = 0; i < values.length; i++) {
        values[i] = start + i;
    }
}

int sum(int[3]* values) {
    int total = 0;
    for (int i = 0; i < values.length; i++) {
        total += values[i];
    }
    return total;
}

void bumpAll(Counter[2]* counters) {
    counters[0].count++;
    counters[1].count += 10;
}

void markCorner(int[2][2]* grid) {
    grid[1][1] = 4;
}

void main() {
    int[3] numbers;
    fill(numbers, 5);
    println(numbers[0]);            // expect: 5
    println(numbers[2]);            // expect: 7
    println(sum(numbers));          // expect: 18

    Counter[2] counters;
    bumpAll(counters);
    bumpAll(counters);
    println(counters[0].count);     // expect: 2
    println(counters[1].count);     // expect: 20

    int[2][2] grid;
    markCorner(grid);
    println(grid[1][1]);            // expect: 4

    fill(numbers, 0);
    println(numbers[1]);            // expect: 1
}
