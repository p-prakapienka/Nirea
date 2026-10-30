// A reference to an array reads and writes the owner's elements (§9.1).

void main() {
    int[3] numbers = {1, 2, 3};
    int[3]* view = numbers;
    view[1] = 20;
    println(view.length);             // expect: 3
    println(numbers[1]);              // expect: 20
}
