// A char prints as its byte; integer-to-char casts keep the low 8 bits (§6.2, §15).

void main() {
    char letter = 'A';
    println(letter);          // expect: A
    println((char) 321);      // expect: A
    println((int) 'z');       // expect: 122
    char high = (char) 255;
    println((uint) high);     // expect: 255
}
