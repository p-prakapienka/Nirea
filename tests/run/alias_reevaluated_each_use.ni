// An alias's expression is evaluated again at each use (§9).

struct Sequence {
    int last;

    int next() {
        last++;
        return last;
    }
}

void main() {
    Sequence sequence;
    alias value = sequence.next();
    println(value);                 // expect: 1
    println(value);                 // expect: 2
    println(sequence.last);         // expect: 2

    int[3] items = {10, 20, 30};
    int position = 0;
    alias current = items[position];
    println(current);               // expect: 10
    position = 2;
    println(current);               // expect: 30

    int count = 1;
    alias doubled = count * 2;
    count = 5;
    println(doubled);               // expect: 10
}
