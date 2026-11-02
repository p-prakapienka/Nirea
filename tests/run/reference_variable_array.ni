// A reference to an array reads and writes the owner's elements (§9.1).

void main() {
    int[] numbers = {1, 2, 3};
    int[]* view = numbers;
    view[1] = 20;
    println(view.length);             // expect: 3
    println(numbers[1]);              // expect: 20
}
