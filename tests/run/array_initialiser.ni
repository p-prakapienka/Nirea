// An array initialiser lists exactly N elements; each element takes its
// type from the array's element type (§6.3, §6.6, Appendix A).

void main() {
    int[] primes = {2, 3, 5};
    println(primes[0]);             // expect: 2
    println(primes[1]);             // expect: 3
    println(primes[2]);             // expect: 5

    String[] words = {"alpha", "beta"};
    println(words[1]);              // expect: beta

    uint8[] bytes = {255, 0};
    println(bytes[0]);              // expect: 255

    float64[] reals = {2.5, -3.0};
    println(reals[1]);              // expect: -3.0

    int[] trailing = {7, 8, 9,};
    println(trailing[2]);           // expect: 9

    int[][] matrix = {{1, 2}, {3, 4}};
    println(matrix[1][0]);          // expect: 3
}
