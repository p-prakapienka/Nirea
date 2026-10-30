// Aliases follow lexical block scope, so sibling blocks can reuse a name (§9).

void main() {
    int[2] values = {1, 2};
    {
        alias item = values[0];
        println(item);              // expect: 1
    }
    {
        alias item = values[1];
        println(item);              // expect: 2
    }
    for (int i = 0; i < values.length; i++) {
        alias element = values[i];
        element = element * 10;
        println(element);
    }
}

// expect: 10
// expect: 20
