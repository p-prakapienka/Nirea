// char is an 8-bit type holding 0 to 255. It supports comparison and explicit
// casts to and from integer types (§6.1, §6.2).

void main() {
    char letter = 'A';
    println((int) letter);          // expect: 65

    char next = (char) 66;
    if (next == 'B') {
        println("cast to char");    // expect: cast to char
    }

    char high = (char) 200;
    println((int) high);            // expect: 200
    println((uint8) high);          // expect: 200
    println((int) '\xFF');          // expect: 255

    int64 code = 97;
    char fromInt64 = (char) code;
    if (fromInt64 == 'a') {
        println("from int64");      // expect: from int64
    }

    if ('a' < 'b') {
        println("ordered");         // expect: ordered
    }
    if (letter != 'a') {
        println("case matters");    // expect: case matters
    }
    if ('\x61' == 'a') {
        println("hex escape");      // expect: hex escape
    }
}
